#include <string>

std::string SplitAtFirstOccurence(std::string& str, const char c)
{
    const size_t indexOfSplitCharacter = str.find(c);
    const std::string substring = str.substr(0, indexOfSplitCharacter);
    str.erase(0, indexOfSplitCharacter + 1);
    return substring;
}

bool ValidateUsername(const std::string& stringToValidate)
{
    for (const auto& c : stringToValidate)
    {
        if (isalpha(c) && c >= 'a' && c <= 'z')
            continue;
        return false;
    }
    return true;
}

std::string GenerateUniqueFilename(const std::string& filename)
{
    const std::string templateStr = filename + "XXXXXX";
    char* name = new char[templateStr.size() + 1];
    std::copy(templateStr.begin(), templateStr.end(), name);
    name[templateStr.size()] = '\0';
    const std::string generatedFile = mkstemp(name) ? name : std::string();
    delete[] name;
    return generatedFile;
}