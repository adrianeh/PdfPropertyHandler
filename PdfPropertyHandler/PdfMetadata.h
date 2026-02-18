#pragma once
#include <string>

struct PdfMetadata {
    std::wstring title;
    std::wstring author;
    std::wstring subject;
    std::wstring keywords;
};

bool ExtractPdfMetadata(const std::wstring& path, PdfMetadata& meta);
