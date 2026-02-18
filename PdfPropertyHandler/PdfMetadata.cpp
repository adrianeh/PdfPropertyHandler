#include "PdfMetadata.h"
#include <fstream>
#include <sstream>

static std::string readFile(const std::wstring& path) {
    std::ifstream f(path, std::ios::binary);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static std::string findStr(const std::string& s, const std::string& key) {
    auto p = s.find(key);
    if (p == std::string::npos) return "";
    p = s.find("(", p);
    if (p == std::string::npos) return "";
    auto e = s.find(")", p + 1);
    if (e == std::string::npos) return "";
    return s.substr(p + 1, e - p - 1);
}

bool ExtractPdfMetadata(const std::wstring& path, PdfMetadata& meta) {
    std::string pdf = readFile(path);
    if (pdf.empty()) return false;

    size_t sx = pdf.rfind("startxref");
    if (sx == std::string::npos) return false;

    size_t line = pdf.find("\n", sx);
    long xref = atol(pdf.substr(line, pdf.find("\n", line + 1) - line).c_str());

    size_t trailer = pdf.find("trailer", xref);
    if (trailer == std::string::npos) return false;

    size_t ds = pdf.find("<<", trailer);
    size_t de = pdf.find(">>", ds);
    std::string tr = pdf.substr(ds, de - ds);

    size_t infoPos = tr.find("/Info");
    if (infoPos == std::string::npos) return false;

    int objNum = 0, gen = 0;
    sscanf(tr.c_str() + infoPos, "/Info %d %d R", &objNum, &gen);

    std::string key = std::to_string(objNum) + " " + std::to_string(gen) + " obj";
    size_t op = pdf.find(key);
    if (op == std::string::npos) return false;

    size_t os = pdf.find("<<", op);
    size_t oe = pdf.find(">>", os);
    std::string dict = pdf.substr(os, oe - os);

    auto toW = [](const std::string& s) {
        return std::wstring(s.begin(), s.end());
    };

    meta.title    = toW(findStr(dict, "/Title"));
    meta.author   = toW(findStr(dict, "/Author"));
    meta.subject  = toW(findStr(dict, "/Subject"));
    meta.keywords = toW(findStr(dict, "/Keywords"));

    return true;
}
