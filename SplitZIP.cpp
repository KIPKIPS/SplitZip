#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "./miniz/miniz.c"

const size_t MAX_ZIP_SIZE = 5 * 1024 * 1024; //zip pkg max size
const std::string OUTPUT_PREFIX = "part"; // prefix of zip file

std::vector<std::string> compress_single_file(const std::string& file_path,const std::string& out_dir_path)
{
    std::ifstream file(file_path, std::ios::binary);
    std::vector<std::string> outputs;
    if (!file)
    {
        std::cerr << "Failed to open file: " << file_path << '\n';
        return outputs;
    }

    //file size
    file.seekg(0, std::ios::end);
    const size_t file_size = file.tellg();
    file.seekg(0);

    //zip pkg count
    int part_index = 0;
    size_t bytes_remaining = file_size;
    bool is_split = file_size >= MAX_ZIP_SIZE;
    
    while (bytes_remaining > 0)
    {
        mz_zip_archive zip_archive = {};
        const std::string::size_type i_pos = file_path.find_last_of('\\') + 1;
        std::string filename = file_path.substr(i_pos, file_path.length() - i_pos);
        // create zip
        std::string filename_without_extension = filename.substr(0, filename.rfind('.'));
        std::string output_zip_path = out_dir_path + '\\' + filename_without_extension + (is_split ? '.' + OUTPUT_PREFIX + std::to_string(part_index + 1) : "") + ".zip" ;
        if (!mz_zip_writer_init_file(&zip_archive, output_zip_path.c_str(), 0))
        {
            std::cerr << "Failed to create zip file: " << output_zip_path << '\n';
            return outputs;
        }
        //zip file size
        size_t current_zip_size = 0;
        const size_t chunk_size = std::min(MAX_ZIP_SIZE, bytes_remaining);

        // read file and add to zip
        std::vector<char> file_data(chunk_size);
        file.read(file_data.data(), static_cast<long>(chunk_size));

        mz_zip_writer_add_mem(&zip_archive, filename.c_str(), file_data.data(), file_data.size(), MZ_BEST_COMPRESSION);

        // update remain bytes and zip size
        bytes_remaining -= chunk_size;
        current_zip_size += chunk_size;

        //close zip
        mz_zip_writer_finalize_archive(&zip_archive);
        mz_zip_writer_end(&zip_archive);
        part_index++;
        
        // std::cout << "output_zip_path:"<<output_zip_path << '\n';
        outputs.push_back(output_zip_path);
    }
    return outputs;
}

void compress_multiple_files(const std::vector<std::string>& files, const std::string& output_path) {
    mz_zip_archive zipArchive = {};
    std::vector<std::string> outputs;
    //init zip
    if (!mz_zip_writer_init_file(&zipArchive, output_path.c_str(), 0)) {
        std::cerr << "Failed to create zip file: " << output_path << '\n';
        return;
    }

    for (const auto& file_path : files) {
        //read file
        std::ifstream file(file_path, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file: " << file_path << '\n';
            continue; //broken file
        }

        // add file to zip
        std::vector<char> fileData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        const std::string::size_type i_pos = file_path.find_last_of('\\') + 1;
        std::string filename = file_path.substr(i_pos, file_path.length() - i_pos);
        mz_zip_writer_add_mem(&zipArchive, filename.c_str(), fileData.data(), fileData.size(), MZ_BEST_COMPRESSION);
    }

    // close zip
    mz_zip_writer_finalize_archive(&zipArchive);
    mz_zip_writer_end(&zipArchive);
}

//all file and partition
void compress_files(std::vector<std::string>& files)
{
    //ascent by file size
    std::sort(files.begin(), files.end(), [](const std::string& path_a, const std::string& path_b)
    {
        std::ifstream file_a(path_a, std::ios::binary);
        std::ifstream file_b(path_b, std::ios::binary);
        if (!file_a || !file_b)
        {
            return false;
        }
        file_a.seekg(0, std::ios::end);
        const size_t size_a = file_a.tellg();
        file_a.seekg(0);
        file_b.seekg(0, std::ios::end);
        const size_t size_b = file_b.tellg();
        file_b.seekg(0);
        // std::cout << path_a <<size_a<< std::endl;
        return size_a < size_b;
    });

    std::vector<std::vector<std::string>> partitions;

    size_t current_zip_size = 0;
    int index = 0;
    const std::vector<std::string> init_paths;
    partitions.push_back(init_paths);
    for (const auto& file_path : files)
    {
        std::ifstream file(file_path, std::ios::binary | std::ios::ate);
        const size_t file_size = file.tellg();
        file.seekg(0);
        if (current_zip_size + file_size > MAX_ZIP_SIZE)
        {
            index++;
            std::vector<std::string> temp_paths;
            partitions.push_back(temp_paths);
            // std::cout << index <<" "<<current_zip_size<< '\n';
            current_zip_size = 0;
        }
        current_zip_size += file_size;
        partitions[index].push_back(file_path);
        std::cout << index <<" "<<file_path<< '\n';
    }
    // std::cout <<"partitions:"<< partitions.capacity()<< '\n';
    index = 0;
    for (const auto& paths : partitions)
    {
        // std::cout << "capacity:"<<paths.capacity() << '\n';
        if (paths.capacity() == 1)
        {
            //single
            std::vector<std::string> rst = compress_single_file(paths[0],R"(C:\Users\kipswang\Desktop\test\outputs)");
            if (rst.capacity() > 0)
            {
                for (const auto& file_path : rst)
                {
                    std::cout << "outs:"<<file_path << '\n';
                }
            }
        }
        else
        {
            //multiple file
            
            auto outs= R"(C:\Users\kipswang\Desktop\test\outputs\multiple_)" + std::to_string(index) + ".zip";
            compress_multiple_files(paths,outs);
        }
        // std::cout << index <<" capacity:"<<paths.capacity()<< '\n';
        // for (const auto& file_path : paths)
        // {
        //     std::cout << file_path << '\n';
        // }
        index++;
    }
}

int main()
{
    std::vector<std::string> files;

    files.emplace_back(R"(C:\Users\kipswang\Desktop\test\5.log)");
    files.emplace_back(R"(C:\Users\kipswang\Desktop\test\2.log)");
    files.emplace_back(R"(C:\Users\kipswang\Desktop\test\3.log)");
    files.emplace_back(R"(C:\Users\kipswang\Desktop\test\1.log)");
    files.emplace_back(R"(C:\Users\kipswang\Desktop\test\4.log)");

    compress_files(files);
    std::cout << "Compression completed." << '\n';

    return 0;
}
