#pragma once

#include <QString>
#include <optional>
#include "CodeEan.hpp"


struct Product
{
    QString name;
    std::optional<CodeEan> codeEan;
    float quantity;
    QString unitCode;
};
