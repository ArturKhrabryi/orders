#pragma once

#include <QString>
#include <QStringList>


struct ProcessResult
{
    bool ok;
    int exitCode;
    QString output;
    QString error;
};

class ProcessRunner
{
public:
    static ProcessResult run(
        const QString& program,
        const QStringList& args = {},
        const QString& workingDir = "",
        int startTimeoutMs = 3000,
        int finishTimeoutMs = 180000
    );
};
