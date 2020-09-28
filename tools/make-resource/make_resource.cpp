#include <chrono>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "cxxopts.hpp"

static std::string ReadFile(std::filesystem::path input_filepath)
{
    std::ifstream      fin(input_filepath, std::ios::binary);
    std::ostringstream oss;

    oss << fin.rdbuf();
    if (!oss.good()) {
        return {};
    }
    return oss.str();
}

static std::string StringToHex(std::string_view in)
{
    std::string out;
    out.reserve(2 * in.size() + 1);

    constexpr char hexchars[] = "0123456789ABCDEF";

    for (char c : in) {
        out.push_back(hexchars[0b1111 & (c >> 4)]);
        out.push_back(hexchars[0b1111 & c]);
    }

    return out;
}

static void WriteFile(
    std::string_view      resource_name,
    std::filesystem::path input_filepath,
    std::filesystem::path output_filepath,
    std::string_view      data)
{
    namespace filesystem = std::filesystem;
    using fmt::format;
    using std::string;

    if (output_filepath.extension().empty()) {
        output_filepath.replace_extension(".cpp");
    }

    const auto id        = StringToHex(resource_name);
    const auto classname = "Resource_" + id;
    const auto varname   = "resource_" + id;

    string buffer;
    buffer.reserve(4096 + 6 * data.size() + 2 * data.size() / 16);

    buffer += "#include \"utils/resource.hpp\"\n\n";
    buffer += format("class {0} final\n", classname);
    buffer += "{\n";
    buffer += "static constexpr unsigned char data[] =\n";
    buffer += "{\n";

    const int max_columns = 16;

    for (std::size_t i = 0, column = 1; i != data.size(); ++i) {
        const unsigned char byte = data[i];
        if (byte == 0x0d) {
            continue;
        }
        buffer += format("{:#04x}", byte);
        const bool should_break = column == max_columns;
        if (i + 1 < data.size()) {
            buffer += should_break ? ",\n" : ", ";
            column = should_break ? 1 : column + 1;
        }
    }

    buffer += "\n};\n\n";
    buffer += "public:\n";
    buffer += classname;
    buffer += "()\n{\n";
    buffer += "  auto& manager = ResourceManager::Instance();\n";
    buffer += "  manager.AddResource(\"";
    buffer += resource_name;
    buffer += "\", data_view(data, sizeof(data)));\n";
    buffer += "}\n};\n\n";

    buffer += format("static {0} {1};\n", classname, varname);

    const auto dirpath = output_filepath.parent_path();

    if (!dirpath.empty() && !filesystem::exists(dirpath)) {
        if (!filesystem::create_directories(dirpath)) {
            throw std::runtime_error(format("Failed to create directory `{0}`", dirpath.string()));
        }
    }

    std::ofstream outfile(output_filepath);

    outfile << buffer;
}

int main(int argc, char** argv)
{
    namespace filesystem = std::filesystem;
    using fmt::format;
    using std::cout;
    using std::runtime_error;
    using std::string;
    using std::string_view;

    try {
        cxxopts::Options options(*argv, "Create a cpp resource file from a binary file");

        options.add_options()("r,resource", "resource name", cxxopts::value<string>())(
            "i,input", "input file path",
            cxxopts::value<string>())("o,output", "output folder path", cxxopts::value<string>());

        auto result = options.parse(argc, argv);

        if (!result["resource"].count() || !result["input"].count() || !result["output"].count()) {
            cout << options.help();
            return EXIT_FAILURE;
        }

        const string_view resource_name   = result["resource"].as<string>();
        const string_view input_filepath  = result["input"].as<string>();
        const string_view output_filepath = result["output"].as<string>();

        if (!filesystem::exists(input_filepath)) {
            throw runtime_error(format("Input file `{0}` does not exist", input_filepath));
        }

        const auto t1 = std::chrono::high_resolution_clock::now();

        string data = ReadFile(input_filepath);
        if (data.empty()) {
            throw runtime_error(format("Failed to read file `{0}`", input_filepath));
        }

        WriteFile(resource_name, input_filepath, output_filepath, data);

        const auto t2 = std::chrono::high_resolution_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

        cout << format("Done. Execution took {0} milliseconds.", duration);
    } catch (const std::exception& e) {
        cout << e.what();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
