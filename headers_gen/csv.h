#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <regex>
#include "outputdocument.h"

namespace csv {

std::vector<std::string> findAllCsv(std::string const &path = "files")
{

    std::vector<std::string> csvFiles;
    std::filesystem::path file_path{path};
    for (const auto &file: std::filesystem::directory_iterator(file_path))
    {
        std::string path = file.path().string();
        if (path.find(".csv") not_eq std::string::npos){
            csvFiles.emplace_back(file.path().string());
        }
    }
    return csvFiles;
}

std::vector<std::string> splitCsvLine(std::string &src) {
    std::vector<std::string> result;
    std::size_t position;
    char separator = ',';

    while ((position = src.find(separator)) not_eq std::string::npos) {
        std::string temp = src.substr(0, position);
        if (temp.empty()) {
            src.erase(0, 1);
            continue;
        }
        else
            result.emplace_back(temp);
        src.erase(0, position + 1);
    }
    result.emplace_back(src);   //FIXME: bad practices

    return result;
}

bool parseCsv(std::string fileName)
{
    std::ifstream file;
    file.open(fileName);

    if (not(file.is_open())) {
        return false;
    }

    std::vector<std::string> splitLine;
    std::string line;

    std::cout << "Perform additional name check? [Y/n]";
    bool check{false};

//    if (std::cin.get() not_eq 'n')
//        check = true;

    OutputDocument * doc = new OutputDocument(fileName, check);

    while (not(file.eof())) {
        getline(file, line);
        splitLine = splitCsvLine(line);
        if (splitLine.size() == 0)
            continue;

        if (splitLine.at(0) == "Device:") {
            getline(file, line);
            splitLine = splitCsvLine(line);
            doc->setDeviceName(splitLine.at(0));
        }

        else if (splitLine.at(0) == "Device states:") {
            getline(file, line);
            splitLine = splitCsvLine(line);
            while (splitLine.at(0) not_eq "" and splitLine.at(0) not_eq "\r") {
                doc->addDeviceState(splitLine.at(0));
                doc->addDeviceStateComment(splitLine.at(1));
                getline(file, line);
                splitLine = splitCsvLine(line);
            }
        }

        else if (splitLine.at(0) == "Verbatim:") {
            getline(file, line);
            splitLine = splitCsvLine(line);
            while (splitLine.at(0) not_eq "" and splitLine.at(0) not_eq "\r") {
                doc->addVerbatim(splitLine.at(0));
                getline(file, line);
                splitLine = splitCsvLine(line);
            }
        }
        else if (splitLine.at(0) == "Enum" or splitLine.at(0) == "enum") {

            doc->addNewEnum(splitLine.at(1));

            getline(file, line);
            splitLine = splitCsvLine(line);

            while (splitLine.at(0) not_eq "" and splitLine.at(0) not_eq "\r") {
                doc->addEnumElement(splitLine.at(0), splitLine.at(1));
                getline(file, line);
                splitLine = splitCsvLine(line);
            }
        }


        else if (splitLine.at(0).find("frame") not_eq std::string::npos) {
            doc->newCanFrame(splitLine.at(1));
            //get frame frequency
            if (splitLine.at(0).find("Asynchronous") not_eq std::string::npos)
                doc->setFrequency(0);
            else {
                std::string freq = splitLine.at(2);
                std::regex rgx("\\d+");
                std::smatch match;

                if (std::regex_search(freq, match, rgx))
                    doc->setFrequency(stoi(match[0]));
            }

            //get frame id
            std::string str = splitLine.at(3);

            if (str.find("0x") not_eq std::string::npos) {   //if id number is present
                str = str.substr(str.find("0x") + 2);
                int id;
                try {
                    id = stoi(str, nullptr, 16);
                }
                  catch (std::invalid_argument const &) {
                    id = 0;
                }
                doc->addID(id);
            }
            else {
                doc->addID(0);
            }

            getline(file, line);
            getline(file, line);
            splitLine = splitCsvLine(line);
            while (splitLine.at(0) not_eq "" and splitLine.at(0) not_eq "\r") {
                doc->addElementToCanFrame(splitLine.at(0), splitLine.at(1), splitLine.at(2));
                getline(file, line);
                splitLine = splitCsvLine(line);
            }
        }
    }

    doc->write();
    delete doc;

    return true;
}

} //namespace csv
