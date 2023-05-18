#include <iostream>
#include <cstdint>

namespace improved_with_annot {
    
namespace internal {
enum class PowerState
{
    Off, 
    WaitStableGround,
	WaitStableAir,
    On,
	WaitOffAir,
};

static PowerState state = PowerState::Off;
static uint32_t start_wait = 0;
} //end namespace

void Trace(const char* msg)
{
	std::cerr << "\n(::" << msg << ")\n";
}

bool fsm_turn_on(double freq, uint32_t curr_ms, bool wow)
{
	const double ON_RPM_GND = 2200;
	const double ON_RPM_AIR = 2400;
	const double MAX_WAIT_RPM = 2300;
	const double OFF_RPM = 1000;
	const uint32_t MAX_WAIT_STABLE_MS = 30000;
	const uint32_t MAX_WAIT_LOW_POWER_MS = 5000;
	using namespace internal;
	//@statechart dia_gen_on2 Диаграмма включения полезной нагрузки 2
	//@Условия:
	//@ - ОБ - Усреднённая частота оборотов (freq)
	//@ - ШО - Комплексный признак ШО (wow)
	//@ - t -  Время со старта блока
	//@ - ВЫСОКИЕ(ВОЗДУХ) = 2400 обороты для включения в воздухе
	//@ - ВЫСОКИЕ(ЗЕМЛЯ) = 2200 обороты для включения в воздухе
	//@ - НИЗКИЕ = 1000 обороты для выключения
	//@ - СТАБИЛЬНЫЕ(воздух) = 2300 обороты для контроля стабильности


	//@[*]-->Off
	switch (state)
	{
	case PowerState::Off: //@Off: Нагрузка выключена
		if (wow && freq >= ON_RPM_GND) { //@Off-->WaitStableGround: ШО И ОБ>=ВЫСОКИЕ(ЗЕМЛЯ)
			state = PowerState::WaitStableGround;
			start_wait = curr_ms;
			Trace("Power off -> wait on gnd");
		} 
		else if (!wow && freq >= ON_RPM_AIR) {  //@Off-->On: НЕ ШО И ОБ>=ВЫСОКИЕ(ВОЗДУХ)
			state = PowerState::On;
			Trace("Power off -> on air");
		}
		break;
	case PowerState::WaitStableGround: //@WaitStableGround: Задержка включения на земле
		if (!wow && (freq >= ON_RPM_AIR) && (curr_ms - start_wait) >= MAX_WAIT_STABLE_MS) { //@WaitStableGround-->On: !ШО И ОБ>=ВЫСОКИЕ(ВОЗДУХ)
			start_wait = 0;
			state = PowerState::On;
			Trace("Power wait on gnd -> on air");
		}
		else if (freq < ON_RPM_GND) { //@WaitStableGround-->Off: ОБ<ВЫСОКИЕ(ЗЕМЛЯ)
			start_wait = 0;
			state = PowerState::Off;
			Trace("Power wait on gnd -> off");
		}
		else if ((curr_ms - start_wait) >= MAX_WAIT_STABLE_MS) { //@WaitStableGround-->On: ОБ>=ВЫСОКИЕ(ВОЗДУХ) И t>=30с
			start_wait = 0;
			state = PowerState::On;
			Trace("Power wait on gnd -> on");
		}
		break;
	case PowerState::On: //@On: Нагрузка включена
		if ( (freq >= OFF_RPM) && (freq < MAX_WAIT_RPM) && (!wow) ) { //@On-->WaitOffAir: НЕ ШО И ОБ>=НИЗКИЕ И ОБ<СТАБИЛЬНЫЕ
			start_wait = curr_ms;
			state = PowerState::WaitOffAir;
			Trace("Power on -> wait off air");
		}
		else if (freq < OFF_RPM) { //@On-->Off: ОБ<НИЗКИЕ
			state = PowerState::Off;
			Trace("Power on -> off low freq");
		}
		break;
	case PowerState::WaitOffAir: //@WaitOffAir: Задержка выключения в воздухе
		if ((freq >= MAX_WAIT_RPM) || wow) { //@WaitOffAir-->On: ОБ>=СТАБИЛЬНЫЕ ИЛИ ШО
			start_wait = 0;
			state = PowerState::On;
			Trace("Power wait off air -> on");
		}
		else if ((freq < OFF_RPM) || ((curr_ms - start_wait) >= MAX_WAIT_LOW_POWER_MS)) { //@WaitOffAir-->Off: ОБ<НИЗКИЕ ИЛИ Т>=5000
			start_wait = 0;
			state = PowerState::Off;
			Trace("Power wait off air -> off");
		}
		break;
	}
	//@endchart
        
    return ( (state == PowerState::On) || (state == PowerState::WaitOffAir) );
}

}//end namespace


int main()
{
    //simple example
    std::cout << "100: N=0, WOW, power=" << improved_with_annot::fsm_turn_on(0, 100, true) << "\n";
    std::cout << "200: N=2200, WOW, power=" << improved_with_annot::fsm_turn_on(2200, 200, true) << "\n";
    std::cout << "30199: N=2200, WOW, power=" << improved_with_annot::fsm_turn_on(2200, 30199, true) << "\n";
    std::cout << "30200: N=2200, WOW, power=" << improved_with_annot::fsm_turn_on(2200, 30200, true) << "\n";
}