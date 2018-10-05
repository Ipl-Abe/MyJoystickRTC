/*!
  @brief A component for accessing a joystick control device
*/

#include <cnoid/Joystick>
#include <rtm/idl/BasicDataTypeSkel.h>
#include <rtm/DataFlowComponentBase.h>
#include <rtm/DataOutPort.h>

class MyJoystickRTC : public RTC::DataFlowComponentBase
{
public:
    MyJoystickRTC(RTC::Manager* manager);
    ~MyJoystickRTC();

    virtual RTC::ReturnCode_t onInitialize();
    virtual RTC::ReturnCode_t onActivated(RTC::UniqueId ec_id);
    virtual RTC::ReturnCode_t onDeactivated(RTC::UniqueId ec_id);
    virtual RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

protected:
    RTC::TimedFloatSeq axes;
    RTC::OutPort<RTC::TimedFloatSeq> axesOut;
    RTC::TimedBooleanSeq buttons;
    RTC::OutPort<RTC::TimedBooleanSeq> buttonsOut;
  
private:
    cnoid::Joystick* joystick; 
    std::string device;
    unsigned int debugLevel;
};


MyJoystickRTC::MyJoystickRTC(RTC::Manager* manager)
    : RTC::DataFlowComponentBase(manager),
      axesOut("axes", axes),
      buttonsOut("buttons", buttons),
      debugLevel(0)
{
    joystick = 0;
}


MyJoystickRTC::~MyJoystickRTC()
{
    if(joystick){
        delete joystick;
    }
}


RTC::ReturnCode_t MyJoystickRTC::onInitialize()
{
    // Bind variables and configuration variable
    bindParameter("device", device, "");
    bindParameter("debugLevel", debugLevel, "0");

    // Set OutPort buffer
    addOutPort("axes", axesOut);
    addOutPort("buttons", buttonsOut);
  
    return RTC::RTC_OK;
}


RTC::ReturnCode_t MyJoystickRTC::onActivated(RTC::UniqueId ec_id)
{
    if(debugLevel > 0){
        std::cout << "MyJoystickRTC::onActivated(" << ec_id << ")" << std::endl;
    }

    if(!joystick){
        joystick = new cnoid::Joystick(device.c_str());
    }
    if(!joystick->isReady()){
        std::cerr << "Joystick device(" << device << ") is not opened" << std::endl;
        return RTC::RTC_ERROR;  
    } else {
        int n = joystick->numAxes();
        axes.data.length(n);
        for(int i=0; i < n; ++i){
            double pos = joystick->getPosition(i);
            if(fabs(pos) < 0.2){
                pos = 0.0;
            }
            axes.data[i] = pos;
        }
        int m = joystick->numButtons();
        buttons.data.length(m);
        for(int i=0; i < m; ++i){
            buttons.data[i] = joystick->getButtonState(i);
        }
        return RTC::RTC_OK;
    }
}


RTC::ReturnCode_t MyJoystickRTC::onDeactivated(RTC::UniqueId ec_id)
{
    if(debugLevel > 0){
        std::cout << "MyJoystickRTC::onDeactivated(" << ec_id << ")" << std::endl;
    }

    if(joystick){
        delete joystick;
        joystick = 0;
    }
    
    return RTC::RTC_OK;
}



RTC::ReturnCode_t MyJoystickRTC::onExecute(RTC::UniqueId ec_id)
{
    if(debugLevel > 0){
        std::cout << "MyJoystickRTC::onExecute(" << ec_id << ")" << std::endl;
    }
    
    joystick->readCurrentState();
    
    if(debugLevel > 0){
        std::cout << "axes:";
    }
    const int n = joystick->numAxes();
    for(int i=0; i < n; ++i){
        axes.data[i] = joystick->getPosition(i);
        if(debugLevel > 0){
            std::cout << axes.data[i];
        }
    }
    if(debugLevel > 0){
        std::cout << ", buttons:";
    }
    const int m = joystick->numButtons();
    for(int i=0; i < m; ++i){
        buttons.data[i] = joystick->getButtonState(i);
        if(debugLevel > 0){
            std::cout << buttons.data[i];
        }
    }
    if(debugLevel > 0){
        std::cout << std::endl;
    }

    axesOut.write();
    buttonsOut.write();

    return RTC::RTC_OK;
}


extern "C"
{
    DLL_EXPORT void MyJoystickRTCInit(RTC::Manager* manager)
    {
        static const char* spec[] = {
            "implementation_id", "MyJoystickRTC",
            "type_name",         "MyJoystickRTC",
            "description",       "Access a joystick control device.",
            "version",           "1.0.0",
            "vendor",            "AIST",
            "category",          "HumanInterfaceDevice",
            "activity_type",     "PERIODIC",
            "kind",              "DataFlowComponent",
            "max_instance",      "1",
            "language",          "C++",
            "lang_type",         "compile",
            // Configuration variables
            "conf.default.device", "",
            "conf.default.debugLevel", "0",
            ""
        };
        coil::Properties profile(spec);
        
        manager->registerFactory(
            profile, RTC::Create<MyJoystickRTC>, RTC::Delete<MyJoystickRTC>);
    }
};
