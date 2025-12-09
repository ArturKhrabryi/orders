#pragma once

#include <QString>
#include <optional>
#include "CodeEan.hpp"


struct Product
{
    QString name;
    std::optional<CodeEan> codeEan;
    QString unitCode;
};

struct ProductFormData
{
    QString name;
    std::optional<CodeEan> codeEan;
    QString unitCode;
    double quantity;
};
