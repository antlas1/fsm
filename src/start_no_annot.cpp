#include <cstdint>
#include <iostream>

namespace start_no_annot {
    
namespace internal {
enum class PowerState
{
    Off, 
    WaitStable,
    On
};

static PowerState state = PowerState::Off;
static uint32_t start_wait = 0;
} //end namespace

bool fsm_turn_on(double freq, uint32_t curr_ms)
{
    const double ON_RPM = 2400;
    const double OFF_RPM = 1000;
    const uint32_t MAX_WAIT_STABLE_MS = 30000;
    using namespace internal;
    
    switch (state)
    {
    case PowerState::Off:
        if (freq >= ON_RPM) {
            state = PowerState::WaitStable;
            start_wait = curr_ms;
        }
        break;
    case PowerState::WaitStable:
        if (freq < ON_RPM)
        {
            start_wait = 0;
            state = PowerState::Off;
        }
        else if ( (curr_ms - start_wait) >= MAX_WAIT_STABLE_MS)
        {
            start_wait = 0;
            state = PowerState::On;
        } 
        break;
    case PowerState::On: 
        if (freq <= OFF_RPM)
        {
            state = PowerState::Off;
        }
        break;
    }
    
    return (state == PowerState::On);
}

}//end namespace

int main()
{
    //simple example
    std::cout << "100: N=0, power=" << start_no_annot::fsm_turn_on(0, 100)<<"\n";
    std::cout << "200: N=2400, power=" << start_no_annot::fsm_turn_on(2400, 200) << "\n";
    std::cout << "30199: N=2400, power=" << start_no_annot::fsm_turn_on(2400, 30199) << "\n";
    std::cout << "30200: N=2400, power=" << start_no_annot::fsm_turn_on(2400, 30200) << "\n";
}