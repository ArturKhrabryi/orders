#include "CodeEan.hpp"
#include <QRegularExpression>


CodeEan::CodeEan(const QString& codeEanText) : value(validateAndBuild(codeEanText)) {}

QString CodeEan::normalize12(QString first12)
{
    for (auto ch : first12.remove(' ')) if (!ch.isDigit())
        throw InvalidEan13CharacterError();

    if (first12.size() != 12)
        throw InvalidFirst12LengthError();
    
    return first12;
}

QString CodeEan::normalize13(QString codeEanText)
{
    for (auto ch : codeEanText.remove(' ')) if (!ch.isDigit())
        throw InvalidEan13CharacterError();

    if (codeEanText.size() != 13)
        throw InvalidEan13LengthError();
    
    return codeEanText;
}

int CodeEan::calculateChecksum(QString first12)
{
    first12 = normalize12(std::move(first12));

    int sum = 0;
    for (int i = 0; i < 12; ++i)
    {
        int digit = first12[i].digitValue();
        sum += (i % 2 == 0) ? digit : digit * 3;
    }

    return (10 - (sum % 10)) % 10;
}

bool CodeEan::hasValidChecksum(const QString& codeEan)
{
    auto normalized = normalize13(codeEan);

    int expected = calculateChecksum(normalized.left(12));
    int actual = normalized[12].digitValue();
    
    return expected == actual;
}

QString CodeEan::validateAndBuild(const QString& codeEanText)
{
    auto codeEan = normalize13(codeEanText);
    if (!hasValidChecksum(codeEan))
        throw InvalidEan13ChecksumError();

    return codeEan;
}
