#ifndef EMP_BENCHMARK_HPP
#define EMP_BENCHMARK_HPP
#include "debug/log.hpp"
#include "utils/time.hpp"
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace emp {
class BenchmarkRegistry {
public:
    static BenchmarkRegistry &get()
    {
        static BenchmarkRegistry s_instance;
        return s_instance;
    }
    void beginRecord(std::string name)
    {
        if(m_stack.empty()) {
            m_children["ROOT"].insert(name);
        } else {
            auto top = m_stack.top();
            m_children[top].insert(name);
        }
        m_stack.push(name);
    }
    void endRecord(std::string name, double value)
    {
        if(!m_measurements.contains(name)) {
            m_measurements[name] = 0.0;
        }
        m_measurements[name] += value;
        auto top_name = m_stack.top();
        if(name != top_name) {
            m_stack_usable = false;
            EMP_LOG_ONCE(DEBUG) << "The stack of benchmark results has been corrupted";
        }
        m_stack.pop();
    }
    std::ostream &print(std::ostream &os = std::cout, size_t indent_size = 4)
    {
        auto rows = m_getRows();
        size_t nameW = 0, timeW = 0;
        for(auto &r : rows) {
            nameW = std::max(nameW, r.name.size() + 2);
            timeW = std::max(timeW, r.time.size() + 2);
        }
        os << "\n";
        const auto pctColor = [](double pct) {
            pct = pct * 0.8;
            const double t = std::max(0.0, std::min(1.0, pct / 100.0));
            const int r = 255;
            const int g = static_cast<int>((1.0 - t) * 255.0 + 0.5);
            std::ostringstream code;
            code << "\033[38;2;" << r << ';' << g << ';' << g << "m";
            return code.str();
        };
        const auto getPctStr = [](double pct, std::string pref = "") {
            std::ostringstream spct;
            spct << pref << std::fixed << std::setprecision(2) << pct << '%';
            return spct.str();
        };
        constexpr const char *pctReset = "\033[0m";
        const auto getPrefix = [&](const Row &r) {
            const std::string indent(static_cast<size_t>(r.depth) * indent_size, ' ');
            std::ostringstream prefix;
            prefix << std::left << std::setw(static_cast<int>(nameW + indent.size())) << indent + r.name + "():" << std::left
                   << std::setw(timeW + 1) << r.time << ' ' << getPctStr(r.pct, "l»");
            return prefix.str();
        };
        size_t prefixW = 0;
        for(auto &r : rows) {
            prefixW = std::max(prefixW, getPrefix(r).size() + 1);
        }
        for(auto &r : rows) {
            const std::string prefix = getPrefix(r);
            os << prefix << std::string(prefixW - prefix.size(), ' ') << pctColor(r.pct) << getPctStr(r.pct_g, "g»") << pctReset
               << '\n';
        }
        return os;
    }
    void reset() { m_measurements.clear(); }

private:
    struct Row {
        std::string name;
        std::string time;
        double pct;
        double pct_g;
        int depth;
    };
    std::vector<Row> m_getRows()
    {
        struct StackInfo {
            double parent_time = 0;
            std::string name;
            int depth = 0;
        };
        std::map<std::string, StackInfo> recordings;
        std::stack<StackInfo> to_process;
        double total_recorded = 0.0;
        for(auto ch : m_children["ROOT"]) {
            total_recorded += m_measurements[ch];
        }
        m_measurements["ROOT"] = total_recorded;
        to_process.push({
            .parent_time = 0.0,
            .name = "ROOT",
            .depth = 0,
        });

        std::vector<Row> rows;

        while(!to_process.empty()) {
            auto info = to_process.top();
            auto local_time = m_measurements[info.name];
            to_process.pop();
            for(auto ch : m_children[info.name]) {
                to_process.push({
                    .parent_time = local_time,
                    .name = ch,
                    .depth = info.depth + 1,
                });
            }
            float percent_parent = local_time / info.parent_time * 100.0;
            float percent_global = local_time / total_recorded * 100.0;

            auto delim = info.name.rfind('/');
            auto pretty_name = info.name.substr(delim + 1);  //  if npos, wraps around

            std::ostringstream ss;
            ss.precision(2);
            double display_time = local_time < 1.0 ? local_time * 1000.0 : local_time;
            auto time_unit = local_time < 1.0 ? "ms" : "s";
            ss << std::fixed << display_time << time_unit;
            rows.push_back({ pretty_name, ss.str(), percent_parent, percent_global, info.depth });
        }
        return rows;
    }
    bool m_stack_usable = true;
    std::unordered_map<std::string, double> m_measurements;
    std::unordered_map<std::string, std::unordered_set<std::string>> m_children;
    std::stack<std::string> m_stack;
};

class BenchmarkStopwatch : private Stopwatch {
    std::string m_name;

public:
    BenchmarkStopwatch(std::string name)
        : m_name(name)
    {
    }
    void start()
    {
        BenchmarkRegistry::get().beginRecord(m_name);
        Stopwatch::start();
    }
    void stop()
    {
        double elapsed = Stopwatch::stop();
        BenchmarkRegistry::get().endRecord(m_name, elapsed);
    }
};
class BenchmarkScoped : private BenchmarkStopwatch {
public:
    BenchmarkScoped(std::string name)
        : BenchmarkStopwatch(name)
    {
        this->start();
    }
    ~BenchmarkScoped() { this->stop(); }
};

#ifndef EMP_BENCHMARK
#define EMP_BENCHMARK 1
#endif

#if EMP_BENCHMARK
#define EMP_CONCAT(a, b)                       EMP_CONCAT_INNER(a, b)
#define EMP_CONCAT_INNER(a, b)                 a##b
#define EMP_UNIQUE_NAME(base)                  EMP_CONCAT(EMP_CONCAT(base, __COUNTER__), EMP)
#define EMP_BENCHMARK_FUNC_INNER(uname)        BenchmarkScoped uname(__FILE__ + std::string("::") + __func__);
#define EMP_BENCHMARK_FUNCn_INNER(name, uname) BenchmarkScoped uname(__FILE__ + std::string("::") + __func__ + "::" + name);

#define EMP_BENCHMARK_FUNCn(name) EMP_BENCHMARK_FUNCn_INNER(name, EMP_UNIQUE_NAME(__func__))
#define EMP_BENCHMARK_FUNC()      EMP_BENCHMARK_FUNC_INNER(EMP_UNIQUE_NAME(__func__))
#else
#define EMP_CONCAT(a, b)
#define EMP_CONCAT_INNER(a, b)
#define EMP_UNIQUE_NAME(base)
#define EMP_BENCHMARK_FUNC_INNER(uname)
#define EMP_BENCHMARK_FUNCn_INNER(name, uname)

#define EMP_BENCHMARK_FUNCn(name)
#define EMP_BENCHMARK_FUNC()
#endif
}

#endif
