#pragma once

#include <QString>
#include <stdexcept>


struct InvalidEan13LengthError : public std::runtime_error
{
    InvalidEan13LengthError() : std::runtime_error("EAN13 must have exactly 13 digits.") {}
};

struct InvalidFirst12LengthError : public std::runtime_error
{
    InvalidFirst12LengthError() : std::runtime_error("First 12 digits must have exactly 12 digits.") {}
};

struct InvalidEan13CharacterError : public std::runtime_error
{
    InvalidEan13CharacterError() : std::runtime_error("EAN13 must contain only digits.") {}
};

struct InvalidEan13ChecksumError : public std::runtime_error
{
    InvalidEan13ChecksumError() : std::runtime_error("EAN13 checksum is invalid") {}
};


class CodeEan
{
public:
    CodeEan(const QString& codeEanText);
    const QString& getValue() const noexcept { return this->value; }
    static int calculateChecksum(QString codeEan);
    [[nodiscard]] static bool hasValidChecksum(const QString& codeEan);

private:
    QString value;

    static QString validateAndBuild(const QString& codeEanText);
    static QString normalize12(QString first12);
    static QString normalize13(QString codeEanText);
};
