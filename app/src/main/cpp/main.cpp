#include "skyline/common.h"
#include "skyline/input/common.h"
#include "skyline/os.h"
#include "skyline/jvm.h"
#include <unistd.h>
#include <csignal>

bool Halt;
jobject Surface;
uint FaultCount;
skyline::GroupMutex jniMtx;
std::shared_ptr<skyline::kernel::OS> os;

void signalHandler(int signal) {
    syslog(LOG_ERR, "Halting program due to signal: %s", strsignal(signal));
    if (FaultCount > 2)
        exit(SIGKILL);
    else
        Halt = true;
    FaultCount++;
}

extern "C" JNIEXPORT void Java_emu_skyline_GameActivity_executeRom(JNIEnv *env, jobject instance, jstring romJstring, jint romType, jint romFd, jint preferenceFd, jint logFd) {
    Halt = false;
    FaultCount = 0;

    std::signal(SIGTERM, signalHandler);
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGINT, signalHandler);
    std::signal(SIGILL, signalHandler);
    std::signal(SIGABRT, signalHandler);
    std::signal(SIGFPE, signalHandler);

    setpriority(PRIO_PROCESS, static_cast<id_t>(getpid()), skyline::constant::PriorityAn.second);

    auto jvmManager = std::make_shared<skyline::JvmManager>(env, instance);
    auto settings = std::make_shared<skyline::Settings>(preferenceFd);
    auto logger = std::make_shared<skyline::Logger>(logFd, static_cast<skyline::Logger::LogLevel>(std::stoi(settings->GetString("log_level"))));
    //settings->List(logger); // (Uncomment when you want to print out all settings strings)

    auto start = std::chrono::steady_clock::now();

    try {
        os = std::make_shared<skyline::kernel::OS>(jvmManager, logger, settings);
        const char *romString = env->GetStringUTFChars(romJstring, nullptr);
        logger->Info("Launching ROM {}", romString);
        env->ReleaseStringUTFChars(romJstring, romString);
        os->Execute(romFd, static_cast<skyline::TitleFormat>(romType));
        logger->Info("Emulation has ended");
    } catch (std::exception &e) {
        logger->Error(e.what());
    } catch (...) {
        logger->Error("An unknown exception has occurred");
    }

    auto end = std::chrono::steady_clock::now();
    logger->Info("Done in: {} ms", (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()));
}

extern "C" JNIEXPORT void Java_emu_skyline_GameActivity_setHalt(JNIEnv *env, jobject instance, jboolean halt) {
    jniMtx.lock(skyline::GroupMutex::Group::Group2);
    Halt = halt;
    jniMtx.unlock();
}

extern "C" JNIEXPORT void Java_emu_skyline_GameActivity_setSurface(JNIEnv *env, jobject instance, jobject surface) {
    jniMtx.lock(skyline::GroupMutex::Group::Group2);
    if (!env->IsSameObject(Surface, nullptr))
        env->DeleteGlobalRef(Surface);
    if (!env->IsSameObject(surface, nullptr))
        Surface = env->NewGlobalRef(surface);
    else
        Surface = surface;
    jniMtx.unlock();
}

extern "C" JNIEXPORT void Java_emu_skyline_GameActivity_setButtonState(JNIEnv *env, jobject instance, jlong id, jint state) {
    skyline::input::npad::NpadButton npadButton;
    npadButton.raw = static_cast<skyline::u64>(id);

    os->input->npad[0]->SetButtonState(npadButton, static_cast<skyline::input::npad::NpadButtonState>(state));
}

extern "C" JNIEXPORT void Java_emu_skyline_GameActivity_setAxisValue(JNIEnv *env, jobject instance, jint id, jint value) {
    os->input->npad[0]->SetAxisValue(static_cast<skyline::input::npad::NpadAxisId>(id), value);
}