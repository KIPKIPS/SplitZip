#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include "./miniz/miniz.c"

const size_t MAX_ZIP_SIZE = 1 * 1024 * 1024; //每个ZIP文件最大1MB
const std::string OUTPUT_PREFIX = "part_"; // 输出ZIP文件的前缀

void compressFilesToZip(const std::vector<std::string>& files) {
    mz_zip_archive zipArchive;
    memset(&zipArchive, 0, sizeof(zipArchive));

    size_t currentZipSize = 0;
    int partIndex = 0;

    for (const auto& filePath : files) {
        
        // 获取文件大小
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file) {
            std::cerr << "Failed to open file: " << filePath << std::endl;
            continue;
        }
        size_t fileSize = file.tellg();
        file.seekg(0);
        // std::cout << "Compression completed." <<filePath<<" "<<fileSize<< std::endl;
        //
        // 如果当前ZIP文件已满，关闭当前ZIP并开始一个新的ZIP
        if (currentZipSize + fileSize > MAX_ZIP_SIZE) {
            if (partIndex > 0) {
                mz_zip_writer_finalize_archive(&zipArchive);
                mz_zip_writer_end(&zipArchive);
            }
            partIndex++;
            std::string outputZipPath = OUTPUT_PREFIX + std::to_string(partIndex) + ".zip";
            if (!mz_zip_writer_init_file(&zipArchive, outputZipPath.c_str(), 0)) {
                std::cerr << "Failed to create zip file: " << outputZipPath << std::endl;
                return;
            }
            currentZipSize = 0; // 重置当前ZIP大小
        }

        //读取文件内容并添加到ZIP文件
        std::vector<char> fileData(fileSize);
        file.read(fileData.data(), fileSize);
        std::string::size_type iPos = filePath.find_last_of('\\') + 1;
        std::string filename = filePath.substr(iPos, filePath.length() - iPos);
        std::cout << "filename:" <<filename<< std::endl;
        mz_zip_writer_add_mem(&zipArchive, filename.c_str(), fileData.data(), fileData.size(), MZ_BEST_COMPRESSION);
        currentZipSize += fileSize; // 更新当前ZIP大小
    }

    // 关闭最后一个ZIP文件
    if (partIndex > 0) {
        mz_zip_writer_finalize_archive(&zipArchive);
        mz_zip_writer_end(&zipArchive);
    }
}

int main(int argc, char* argv[])
{
    
    std::vector<std::string> files;

    files.push_back("C:\\Users\\kipswang\\Desktop\\test\\SpeedGame_Windows_Dev_2238544.log");
    // files.push_back("C:\\Users\\kipswang\\Desktop\\test\\SpeedGame_Windows_Dev_test.log");
    // files.push_back("C:\\Users\\kipswang\\Desktop\\test\\SpeedGame_Windows_Dev_test2.log");
    // files.push_back("C:\\Users\\kipswang\\Desktop\\test\\SpeedGame_Windows_Dev_test3.log");
    // files.push_back("C:\\Users\\kipswang\\Desktop\\test\\SpeedGame_Windows_Dev_test5.log");

    compressFilesToZip(files);
    std::cout << "Compression completed." << std::endl;

    return 0;
}
