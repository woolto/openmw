#include "translation.hpp"
#include <components/misc/stringops.hpp>

#include <fstream>

namespace Translation
{
    void Storage::loadTranslationData(const Files::Collections& dataFileCollections,
                                      const std::string& esmFileName)
    {
        std::string esmNameNoExtension(Misc::StringUtils::lowerCase(esmFileName));
        //changing the extension
        size_t dotPos = esmNameNoExtension.rfind('.');
        if (dotPos != std::string::npos)
            esmNameNoExtension.resize(dotPos);

        loadData(mCellNamesTranslations, esmNameNoExtension, ".cel", dataFileCollections);
        loadData(mPhraseForms, esmNameNoExtension, ".top", dataFileCollections);
        loadData(mTopicIDs, esmNameNoExtension, ".mrk", dataFileCollections);
    }

    void Storage::loadData(ContainerType& container,
                           const std::string& fileNameNoExtension,
                           const std::string& extension,
                           const Files::Collections& dataFileCollections)
    {
        std::string fileName = fileNameNoExtension + extension;

        if (dataFileCollections.getCollection (extension).doesExist (fileName))
        {
            std::string path = dataFileCollections.getCollection (extension).getPath (fileName).string();

            std::ifstream stream (path);

            if (!stream.is_open())
                throw std::runtime_error ("failed to open translation file: " + fileName);

            loadDataFromStream(container, stream);
        }
    }

    void Storage::loadDataFromStream(ContainerType& container, std::istream& stream)
    {
        std::string line;
        while (!stream.eof())
        {
            std::getline( stream, line );
            if (!line.empty() && *line.rbegin() == '\r')
              line.resize(line.size() - 1);

            if (!line.empty())
            {
                char* buffer = ToUTF8::getBuffer(line.size() + 1);
                //buffer has at least line.size() + 1 bytes, so it must be safe
                strcpy(buffer, line.c_str());
                line = ToUTF8::getUtf8(mEncoding);

                size_t tab_pos = line.find('\t');
                if (tab_pos != std::string::npos && tab_pos > 0 && tab_pos < line.size() - 1)
                {
                    std::string key = line.substr(0, tab_pos);
                    std::string value = line.substr(tab_pos + 1);

                    if (!key.empty() && !value.empty())
                        container.insert(std::make_pair(key, value));
                }
            }
        }
    }

    std::string Storage::translateCellName(const std::string& cellName) const
    {
        std::map<std::string, std::string>::const_iterator entry =
            mCellNamesTranslations.find(cellName);

        if (entry == mCellNamesTranslations.end())
            return cellName;

        return entry->second;
    }

    std::string Storage::topicID(const std::string& phrase) const
    {
        std::string result = topicStandardForm(phrase);

        //seeking for the topic ID
        std::map<std::string, std::string>::const_iterator topicIDIterator =
            mTopicIDs.find(result);

        if (topicIDIterator != mTopicIDs.end())
            result = topicIDIterator->second;

        return result;
    }

    std::string Storage::topicStandardForm(const std::string& phrase) const
    {
        std::map<std::string, std::string>::const_iterator phraseFormsIterator =
            mPhraseForms.find(phrase);

        if (phraseFormsIterator != mPhraseForms.end())
            return phraseFormsIterator->second;
        else
            return phrase;
    }

    void Storage::setEncoding (const ToUTF8::FromType& encoding)
    {
        mEncoding = encoding;
    }

    bool Storage::hasTranslation() const
    {
        return !mCellNamesTranslations.empty() ||
               !mTopicIDs.empty() ||
               !mPhraseForms.empty();
    }
}
