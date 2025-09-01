#pragma once

#include <functional>


class ScopeExit
{
private:
    std::function<void()> func;
    
public:
    explicit ScopeExit(std::function<void()> func) : func(std::move(func)) {}
    ~ScopeExit() noexcept { try { if (func) func(); } catch (...) {} }
    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
    ScopeExit(ScopeExit&& other) noexcept : func(std::move(other.func)) { other.func = nullptr; }
    ScopeExit& operator=(ScopeExit&& other) = delete;

    void release() noexcept { func = nullptr; } 
};
