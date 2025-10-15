#include "ProcessRunner.hpp"
#include <QCoreApplication>
#include <QProcess>


ProcessResult ProcessRunner::run(const QString& program, const QStringList& args, const QString& workingDir, int startTimeoutMs, int finishTimeoutMs)
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(args);
    process.setProcessChannelMode(QProcess::MergedChannels);
    if (!workingDir.isEmpty())
        process.setWorkingDirectory(workingDir);

    process.start();
    if (!process.waitForStarted(startTimeoutMs))
        return { false, -1, "", process.errorString() };

    if (!process.waitForFinished(finishTimeoutMs))
        return { false, -1, "", QCoreApplication::translate("ProcessRunner", "Execution timeout") };

    const auto out = QString::fromLocal8Bit(process.readAllStandardOutput());
    const bool ok = (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0);
    
    return { ok, process.exitCode(), out, "" };
}

