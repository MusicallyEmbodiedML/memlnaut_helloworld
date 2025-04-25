#include "src/memllib/interface/InterfaceBase.hpp"
#include "src/memllib/audio/AudioAppBase.hpp"
#include "src/memllib/audio/AudioDriver.hpp"
#include "src/memllib/hardware/memlnaut/MEMLNaut.hpp"
#include <memory>


class HelloWorldInterface : public InterfaceBase
{
public:
    HelloWorldInterface() : InterfaceBase() {}

    void setup(size_t n_inputs, size_t n_outputs) override
    {
        InterfaceBase::setup(n_inputs, n_outputs);
        // Additional setup code specific to HelloWorldInterface
    }
};

class HelloWorldAudioApp : public AudioAppBase
{
public:
    HelloWorldAudioApp() : AudioAppBase() {}

    stereosample_t Process(const stereosample_t x) override
    {
        // Process audio sample
        return x;
    }

    void Setup(float sample_rate, std::shared_ptr<InterfaceBase> interface) override
    {
        AudioAppBase::Setup(sample_rate, interface);
        // Additional setup code specific to HelloWorldAudioApp
    }

    void ProcessParams(const std::vector<float>& params) override
    {
        // Process parameters received from the queue
        for (const auto& param : params) {
            Serial.println(param);
        }
    }
};


// Global objects
std::shared_ptr<HelloWorldInterface> helloWorldInterface;
std::shared_ptr<HelloWorldAudioApp> helloWorldAudioApp;


// Inter-core communication
volatile bool core_0_ready = false;
volatile bool core_1_ready = false;
volatile bool serial_ready = false;


void setup()
{
    Serial.begin(115200);
    delay(1000); // Wait for serial to initialize
    serial_ready = 1;

    // Setup board
    MEMLNaut::Initialize();

    // Setup interface
    helloWorldInterface = std::make_shared<HelloWorldInterface>();
    helloWorldInterface->setup(2, 2); // Example: 2 inputs, 2 outputs

    core_0_ready = true; // Indicate core 0 is ready
    while (!core_1_ready) {}
}

void loop()
{
    MEMLNaut::Instance()->loop();
    static int blip_counter = 0;
    if (blip_counter++ > 100) {
        blip_counter = 0;
        Serial.println(".");
    }
    delay(10); // Add a small delay to avoid flooding the serial output
}

void setup1()
{
    while (!serial_ready) {} // Wait for serial to be ready

    helloWorldAudioApp = std::make_shared<HelloWorldAudioApp>();
    while (!helloWorldInterface);  // Wait for interface to be ready

    helloWorldAudioApp->Setup(AudioDriver::GetSampleRate(),
            helloWorldInterface);

    // Start audio driver
    AudioDriver::Setup();

    core_1_ready = true; // Indicate core 1 is ready
    while (!core_0_ready) {}
}

void loop1()
{
    // Audio app parameter processing loop
    helloWorldAudioApp->loop();
}
