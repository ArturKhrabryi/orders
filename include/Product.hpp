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

    operator QString() const
    {
        return name + " " + (codeEan.has_value() ? codeEan->getValue() : "-") + " " + QString::number(quantity) + " " + unitCode;
    }
};
