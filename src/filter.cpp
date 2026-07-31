#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#define WINDOWS_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#pragma warning(disable : 4819)
#endif

#include <Exceptions.h>
#include <ImageIO.h>
#include <ImagesCPU.h>
#include <ImagesNPP.h>

#include <cuda_runtime.h>
#include <npp.h>

#include <helper_cuda.h>
#include <helper_string.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;


bool printfNPPinfo(int argc, char *argv[])
{
    const NppLibraryVersion *libVer = nppGetLibVersion();

    printf("NPP Library Version %d.%d.%d\n",
           libVer->major, libVer->minor, libVer->build);

    int driverVersion, runtimeVersion;
    cudaDriverGetVersion(&driverVersion);
    cudaRuntimeGetVersion(&runtimeVersion);

    printf("  CUDA Driver  Version: %d.%d\n",
           driverVersion / 1000,
           (driverVersion % 100) / 10);

    printf("  CUDA Runtime Version: %d.%d\n",
           runtimeVersion / 1000,
           (runtimeVersion % 100) / 10);

    return checkCudaCapabilities(1, 0);
}


bool hasPgmExtension(const fs::path &path)
{
    std::string ext = path.extension().string();

    std::transform(
        ext.begin(),
        ext.end(),
        ext.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

    return ext == ".pgm";
}


bool processImage(const fs::path &inputPath,
                  const fs::path &outputDir)
{
    std::cout << "\n----------------------------------------\n";
    std::cout << "Processing: " << inputPath << std::endl;

    try
    {
        // ----------------------------------------
        // Load source image
        // ----------------------------------------
        npp::ImageCPU_8u_C1 oHostSrc;
        npp::loadImage(inputPath.string(), oHostSrc);

        std::cout << "Image size: "
                  << oHostSrc.width() << " x "
                  << oHostSrc.height() << std::endl;

        // ----------------------------------------
        // Copy image from host -> GPU
        // ----------------------------------------
        npp::ImageNPP_8u_C1 oDeviceSrc(oHostSrc);

        // ----------------------------------------
        // Box filter parameters
        // ----------------------------------------
        NppiSize oMaskSize = {5, 5};

        NppiSize oSrcSize = {
            static_cast<int>(oDeviceSrc.width()),
            static_cast<int>(oDeviceSrc.height())
        };

        NppiPoint oSrcOffset = {0, 0};

        NppiSize oSizeROI = {
            static_cast<int>(oDeviceSrc.width()),
            static_cast<int>(oDeviceSrc.height())
        };

        // ----------------------------------------
        // Allocate destination image on GPU
        // ----------------------------------------
        npp::ImageNPP_8u_C1 oDeviceDst(
            oSizeROI.width,
            oSizeROI.height
        );

        // Center of 5x5 filter
        NppiPoint oAnchor = {
            oMaskSize.width / 2,
            oMaskSize.height / 2
        };

        // ----------------------------------------
        // Run box filter on GPU
        // ----------------------------------------
        NPP_CHECK_NPP(
            nppiFilterBoxBorder_8u_C1R(
                oDeviceSrc.data(),
                oDeviceSrc.pitch(),
                oSrcSize,
                oSrcOffset,
                oDeviceDst.data(),
                oDeviceDst.pitch(),
                oSizeROI,
                oMaskSize,
                oAnchor,
                NPP_BORDER_REPLICATE
            )
        );

        // ----------------------------------------
        // Copy result GPU -> host
        // ----------------------------------------
        npp::ImageCPU_8u_C1 oHostDst(oDeviceDst.size());

        oDeviceDst.copyTo(
            oHostDst.data(),
            oHostDst.pitch()
        );

        // ----------------------------------------
        // Construct output filename
        // ----------------------------------------
        fs::path outputPath =
            outputDir /
            (inputPath.stem().string() + "_boxFilter.pgm");

        // ----------------------------------------
        // Save result
        // ----------------------------------------
        saveImage(outputPath.string(), oHostDst);

        std::cout << "Saved: " << outputPath << std::endl;

        // Same cleanup approach as your original program
        nppiFree(oDeviceSrc.data());
        nppiFree(oDeviceDst.data());

        return true;
    }
    catch (npp::Exception &e)
    {
        std::cerr << "NPP error while processing "
                  << inputPath << ":\n"
                  << e << std::endl;
        return false;
    }
    catch (std::exception &e)
    {
        std::cerr << "Error while processing "
                  << inputPath << ":\n"
                  << e.what() << std::endl;
        return false;
    }
    catch (...)
    {
        std::cerr << "Unknown error while processing "
                  << inputPath << std::endl;
        return false;
    }
}


int main(int argc, char *argv[])
{
    printf("%s Starting...\n\n", argv[0]);

    try
    {
        // ----------------------------------------
        // Select CUDA device
        // ----------------------------------------
        int dev = findCudaDevice(argc, (const char **)argv);

        int dev_chosen;
        cudaGetDevice(&dev_chosen);

        std::cout << "Current device: "
                  << dev_chosen << std::endl;

        int count;
        cudaGetDeviceCount(&count);

        std::cout << "Number of GPUs: "
                  << count << std::endl;

        for (int i = 0; i < count; i++)
        {
            cudaDeviceProp prop;
            cudaGetDeviceProperties(&prop, i);

            std::cout << "Device " << i << ": "
                      << prop.name << std::endl;
        }

        // ----------------------------------------
        // Print CUDA / NPP information
        // ----------------------------------------
        if (!printfNPPinfo(argc, argv))
        {
            return EXIT_SUCCESS;
        }

        // ----------------------------------------
        // Read input directory
        // ----------------------------------------
        char *inputPathArg = nullptr;

        if (!checkCmdLineFlag(argc, (const char **)argv, "input"))
        {
            std::cerr
                << "Usage:\n"
                << "  ./trial --input pgm_files --output output\n";

            return EXIT_FAILURE;
        }

        getCmdLineArgumentString(
            argc,
            (const char **)argv,
            "input",
            &inputPathArg
        );

        fs::path inputDir(inputPathArg);

        // ----------------------------------------
        // Output directory
        // ----------------------------------------
        fs::path outputDir = "output";

        char *outputPathArg = nullptr;

        if (checkCmdLineFlag(argc, (const char **)argv, "output"))
        {
            getCmdLineArgumentString(
                argc,
                (const char **)argv,
                "output",
                &outputPathArg
            );

            outputDir = outputPathArg;
        }

        // ----------------------------------------
        // Validate input directory
        // ----------------------------------------
        if (!fs::exists(inputDir))
        {
            std::cerr << "Input directory does not exist: "
                      << inputDir << std::endl;

            return EXIT_FAILURE;
        }

        if (!fs::is_directory(inputDir))
        {
            std::cerr << "Input path is not a directory: "
                      << inputDir << std::endl;

            return EXIT_FAILURE;
        }

        // ----------------------------------------
        // Create output directory
        // ----------------------------------------
        fs::create_directories(outputDir);

        std::cout << "\nInput directory : "
                  << fs::absolute(inputDir) << std::endl;

        std::cout << "Output directory: "
                  << fs::absolute(outputDir) << std::endl;

        // ----------------------------------------
        // Find all PGM files
        // ----------------------------------------
        std::vector<fs::path> files;

        for (const auto &entry :
             fs::directory_iterator(inputDir))
        {
            if (entry.is_regular_file() &&
                hasPgmExtension(entry.path()))
            {
                files.push_back(entry.path());
            }
        }

        // Sort files so processing order is deterministic
        std::sort(files.begin(), files.end());

        std::cout << "\nFound "
                  << files.size()
                  << " PGM files.\n";

        if (files.empty())
        {
            std::cerr << "No .pgm files found in "
                      << inputDir << std::endl;

            return EXIT_FAILURE;
        }

        // ----------------------------------------
        // Process all images
        // ----------------------------------------
        int successCount = 0;
        int failureCount = 0;

        for (size_t i = 0; i < files.size(); ++i)
        {
            std::cout << "\n[" << (i + 1)
                      << "/" << files.size() << "]"
                      << std::endl;

            if (processImage(files[i], outputDir))
            {
                ++successCount;
            }
            else
            {
                ++failureCount;
            }
        }

        // ----------------------------------------
        // Summary
        // ----------------------------------------
        std::cout << "\n========================================\n";
        std::cout << "Processing complete\n";
        std::cout << "Successful: " << successCount << "\n";
        std::cout << "Failed:     " << failureCount << "\n";
        std::cout << "Total:      " << files.size() << "\n";
        std::cout << "========================================\n";

        return (failureCount == 0)
                   ? EXIT_SUCCESS
                   : EXIT_FAILURE;
    }
    catch (npp::Exception &e)
    {
        std::cerr
            << "Program error:\n"
            << e << std::endl;

        return EXIT_FAILURE;
    }
    catch (std::exception &e)
    {
        std::cerr
            << "Program error:\n"
            << e.what() << std::endl;

        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr
            << "Unknown program error."
            << std::endl;

        return EXIT_FAILURE;
    }
}
