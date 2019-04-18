/*
Copyright (c) 2019 Onur Rauf Bingol

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "json2on.h"


bool json2on(std::string &jsonString, Config &cfg, std::string &fileName)
{
    // Copy string to the stream
    std::stringstream ss;
    ss << jsonString;

    // Convert string to JSON object
    Json::Value root;
    Json::CharReaderBuilder rbuilder;
    std::string jsonErrors;
    if (!Json::parseFromStream(rbuilder, ss, &root, &jsonErrors))
    {
        if (!cfg.silent())
            std::cout << "[ERROR] Failed to parse JSON string: " << jsonErrors << std::endl;
        return false;
    }

    // Start modeler
    initializeRwExt();

    // Create model
    ONX_Model model;

    // Read shape data from JSON
    for (auto d : root["shape"]["data"])
    {
        if (root["shape"]["type"].asString() == "curve")
        {
            ON_NurbsCurve *geom;
            constructCurveData(d, cfg, geom);
            model.AddManagedModelGeometryComponent(geom, nullptr);
        }
        if (root["shape"]["type"].asString() == "surface")
        {
            ON_Brep *geom;
            constructSurfaceData(d, cfg, geom);
            model.AddManagedModelGeometryComponent(geom, nullptr);
        }
    }

    // Write model to the file (version = 50)
    std::string fnameSave = fileName.substr(0, fileName.find_last_of(".")) + ".3dm";
    bool saveStatus = model.Write(fnameSave.c_str(), 50);

    // Stop modeler
    finalizeRwExt();

    // Print message if save is success
    if (saveStatus && !cfg.silent())
        std::cout << "[SUCCESS] Geometry data was saved to file '" << fnameSave << "' successfully" << std::endl;
    return saveStatus;
}


bool json2on_run(std::string &fileName, Config &cfg)
{
    // Open JSON file
    std::ifstream fp(fileName);
    if (!fp)
    {
        if (!cfg.silent())
            std::cout << "[ERROR] Cannot open file '" << fileName << "' for reading" << std::endl;
        return false;
    }

    // Read file to a string
    std::string jsonString;
    fp >> jsonString;

    if (json2on(jsonString, cfg, fileName))
        return true;
    return false;
}