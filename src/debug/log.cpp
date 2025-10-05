#include "log.hpp"
#include <memory>
#include "gui/console.hpp"
namespace emp {

LogLevel Log::s_reporting_level = DEBUG3;
const std::thread::id Log::s_main_thread = std::this_thread::get_id();

std::vector<std::unique_ptr<LogOutput>> Log::s_out;
//
void Log::enableLoggingToCerr()
{
    s_out.emplace_back(std::move(std::make_unique<Output2Term>()));
}
void Log::enableLoggingToFlie(std::string filename)
{
    s_out.emplace_back(std::make_unique<Output2FILE>(filename));
}
void Log::addLoggingOutput(std::unique_ptr<LogOutput> &&output)
{
    s_out.emplace_back(std::move(output));
}

std::mutex Log::s_out_lock;

Log::Log() { }

std::ostringstream &Log::Get(LogLevel level)
{
    m_level = level;
    os << NowTime();
    if(s_main_thread != std::this_thread::get_id()) {
        os << ", thread: " << std::this_thread::get_id();
    }
    os << ", ";
    return os;
}

Log::~Log()
{
    os << std::endl;
    std::lock_guard<std::mutex> lock(s_out_lock);
    for(auto &output : s_out) {
        output->Output(os.str(), m_level);
    }
}

std::string Log::ToString(LogLevel level, bool usingColors)
{
    static const char *const colors[] = { "\033[0;31m", "\033[0;33m", "\033[1;37m", "\033[0;36m", "\033[0;36m",
                                          "\033[0;36m", "\033[0;36m", "\033[0;36m", "\033[0;36m" };
    static const char *reset = "\033[0m";
    static const char *const buffer[] = { "ERROR", "WARNING", "INFO", "DEBUG", "DEBUG1", "DEBUG2", "DEBUG3", "DEBUG4", "DEBUG5" };
    if(usingColors) {
        return std::string(colors[level]) + buffer[level] + reset;
    }
    return buffer[level];
}

std::string FormatTime(std::chrono::system_clock::time_point tp)
{
    std::stringstream ss;
    auto t = std::chrono::system_clock::to_time_t(tp);
    auto tp2 = std::chrono::system_clock::from_time_t(t);
    if(tp2 > tp) {
        t = std::chrono::system_clock::to_time_t(tp - std::chrono::seconds(1));
    }
    ss << std::put_time(std::localtime(&t), "%T")  //"%Y-%m-%d %T"
       << "." << std::setfill('0') << std::setw(6)
       << (std::chrono::duration_cast<std::chrono::microseconds>(tp.time_since_epoch()).count() % 1000000);
    return ss.str();
}
std::chrono::system_clock::time_point starting_log = std::chrono::system_clock::time_point::min();
std::string RunningSeconds(std::chrono::system_clock::time_point tp)
{
    auto dur = tp - starting_log;
    double secs = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count()) / 1e9;
    return std::to_string(secs);
}
std::string NowTime()
{
    if(starting_log == std::chrono::system_clock::time_point::min()) {
        starting_log = std::chrono::system_clock::now();
    }
    //  return FormatTime(std::chrono::system_clock::now());
    return RunningSeconds(std::chrono::system_clock::now());
}

}  //  namespace emp
