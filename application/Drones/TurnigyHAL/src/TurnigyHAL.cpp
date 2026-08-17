/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// System stuff
//
#include <iterator>

//
// Base stuff
//

//
// Target stuff
//
#include <Target/Target.h>
#include <Target/SystemTick.h>
#include <Target/Main.h>

//
// CMSIS stuff
//
#include <arm_math.h>

//
// System stuff
//

//
// Logic stuff
//
#include <Logic/Host.h>
#include <Logic/MainLoop.h>
#include <Logic/PulseModulation.h>

//
// sensors stuff
//
#include <Sensors/MPU6050.h>

//
// Control stuff
//
#include <Control/IMU.h>
#include <Control/PID.h>

//
// Local stuff
//
using namespace std;
using namespace RAM::Logic::Host::Console;
using namespace RAM::Logic::Host::CommandReply;
using namespace RAM::Logic::PulseModulation;

const float MPI = 3.1415927410125732421875f;
const float MPI_2 = PI / 2;
#define SAMPLE_PERIOD_MS 2

#define PlainList1(KeyScope, x) KeyScope::x
#define PlainList2(KeyScope, x, ...) KeyScope::x, PlainList1(KeyScope, __VA_ARGS__)
#define PlainList3(KeyScope, x, ...) KeyScope::x, PlainList2(KeyScope, __VA_ARGS__)
#define PlainList4(KeyScope, x, ...) KeyScope::x, PlainList3(KeyScope, __VA_ARGS__)
#define PlainList5(KeyScope, x, ...) KeyScope::x, PlainList4(KeyScope, __VA_ARGS__)
#define PlainList6(KeyScope, x, ...) KeyScope::x, PlainList5(KeyScope, __VA_ARGS__)
#define PlainList7(KeyScope, x, ...) KeyScope::x, PlainList6(KeyScope, __VA_ARGS__)
#define PlainList8(KeyScope, x, ...) KeyScope::x, PlainList7(KeyScope, __VA_ARGS__)
#define PlainList9(KeyScope, x, ...) KeyScope::x, PlainList8(KeyScope, __VA_ARGS__)
#define PlainList10(KeyScope, x, ...) KeyScope::x, PlainList9(KeyScope, __VA_ARGS__)
#define PlainList11(KeyScope, x, ...) KeyScope::x, PlainList10(KeyScope, __VA_ARGS__)
#define PlainList12(KeyScope, x, ...) KeyScope::x, PlainList11(KeyScope, __VA_ARGS__)
#define PlainList13(KeyScope, x, ...) KeyScope::x, PlainList12(KeyScope, __VA_ARGS__)
#define PlainList14(KeyScope, x, ...) KeyScope::x, PlainList13(KeyScope, __VA_ARGS__)
#define PlainList15(KeyScope, x, ...) KeyScope::x, PlainList14(KeyScope, __VA_ARGS__)
#define SwitchCase1(KeyScope, count, x) case KeyScope::x: return m_objects[count - 1];
#define SwitchCase2(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 2]; SwitchCase1(KeyScope, count, __VA_ARGS__)
#define SwitchCase3(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 3]; SwitchCase2(KeyScope, count, __VA_ARGS__)
#define SwitchCase4(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 4]; SwitchCase3(KeyScope, count, __VA_ARGS__)
#define SwitchCase5(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 5]; SwitchCase4(KeyScope, count, __VA_ARGS__)
#define SwitchCase6(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 6]; SwitchCase5(KeyScope, count, __VA_ARGS__)
#define SwitchCase7(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 7]; SwitchCase6(KeyScope, count, __VA_ARGS__)
#define SwitchCase8(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 8]; SwitchCase7(KeyScope, count, __VA_ARGS__)
#define SwitchCase9(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 9]; SwitchCase8(KeyScope, count, __VA_ARGS__)
#define SwitchCase10(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 10]; SwitchCase9(KeyScope, count, __VA_ARGS__)
#define SwitchCase11(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 11]; SwitchCase10(KeyScope, count, __VA_ARGS__)
#define SwitchCase12(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 12]; SwitchCase11(KeyScope, count, __VA_ARGS__)
#define SwitchCase13(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 13]; SwitchCase11(KeyScope, count, __VA_ARGS__)
#define SwitchCase14(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 14]; SwitchCase11(KeyScope, count, __VA_ARGS__)
#define SwitchCase15(KeyScope, count, x, ...) case KeyScope::x: return m_objects[count - 15]; SwitchCase11(KeyScope, count, __VA_ARGS__)
#define VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(X,##__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define PlainListVARARG_IMPL2(base, KeyScope, count, ...) base##count(KeyScope, __VA_ARGS__)
#define SwitchCaseVARARG_IMPL2(base, KeyScope, count, ...) base##count(KeyScope, count, __VA_ARGS__)
#define VARARG_IMPL(base, KeyScope, count, ...) base##VARARG_IMPL2(base, KeyScope, count, __VA_ARGS__)
#define VARARG(base, KeyScope, ...) VARARG_IMPL(base, KeyScope, VA_NARGS(__VA_ARGS__), __VA_ARGS__)
#define BuildPlainList(KeyScope, ...) VARARG(PlainList, KeyScope, __VA_ARGS__)
#define BuildSwitchCase(KeyScope, ...) VARARG(SwitchCase, KeyScope, __VA_ARGS__)
#define BuildBracketOperator(KeyType, VarType, ...)                     \
    VarType & operator[](const KeyType key)                             \
    {                                                                   \
        switch ( key )                                                  \
        {                                                               \
            BuildSwitchCase(KeyType, __VA_ARGS__);                      \
        default: error(); break;                                        \
        };                                                              \
        return m_objects[0];                                            \
    }
#define BuildEnumeratedContainer(Name, Enum, ...)                       \
    template<typename VarType>                                          \
    struct Name                                                         \
    {                                                                   \
        Name(VarType *ptr = nullptr)                                    \
        {                                                               \
            m_objects = (VarType *)(ptr == nullptr ? malloc(sizeof(VarType) * VA_NARGS(__VA_ARGS__)) : ptr); \
        }                                                               \
                                                                        \
        const initializer_list<Enum> m_list { BuildPlainList(Enum, __VA_ARGS__) }; \
                                                                        \
        BuildBracketOperator(Enum, VarType, __VA_ARGS__);               \
                                                                        \
        VarType *m_objects;                                             \
    };

enum class DroneVariables : unsigned char {ROLL, PITCH, YAW, THROTTLE, CTRLTYPE, CALIBWAIT, NOP1, NOP2, U1, U2, U3, U4};

int error()
{
    return 0;
}

BuildEnumeratedContainer(
    RemoteControlChannels, DroneVariables, ROLL, PITCH, THROTTLE, YAW, CTRLTYPE, CALIBWAIT, NOP1);
BuildEnumeratedContainer(
    AttitudeAxes, DroneVariables, ROLL, PITCH, YAW);
BuildEnumeratedContainer(
    ControlActions, DroneVariables, THROTTLE, ROLL, PITCH, YAW);
BuildEnumeratedContainer(
    MotorActions, DroneVariables, U1, U2, U3, U4);
BuildEnumeratedContainer(
    GyroVelocities, DroneVariables, PITCH, ROLL, YAW);

float mpuOutput[19];

RemoteControlChannels<float> rcTimings;
AttitudeAxes<float> rcTimings0;
RemoteControlChannels<bool> rcInverted;
AttitudeAxes<float> angles;
AttitudeAxes<RAM::Control::PID> pids;
ControlActions<float> references;
ControlActions<float> uattitude;
MotorActions<float> umotor;
GyroVelocities<float> gyro(mpuOutput + 3);

// F/0/1
RAM::Logic::PulseModulation::PWM::Encoder pwm0(
    RAM::Target::PWM::GeneratorId::PWMGENID6,
    RAM::Target::PWM::SelectId::PWMBOTH,
    2500,
    1000,
    2000,
    1); /* start at maximum in case of ESC calibration */

// F/2/3
RAM::Logic::PulseModulation::PWM::Encoder pwm1(
    RAM::Target::PWM::GeneratorId::PWMGENID7,
    RAM::Target::PWM::SelectId::PWMBOTH,
    2500,
    1000,
    2000,
    1); /* start at maximum in case of ESC calibration */

//PPM::Decoder<RAM::Target::Timer::ModuleId::TIMID1> ppm(7, 18000, 2070);
PPM::Decoder<RAM::Target::Timer::ModuleId::TIMID1> *ppm;

RAM::Device::MPU6050 mpu6050(MPU6050_ADDRESS_AD0_LOW, mpuOutput);
 
RAM::Control::IMU::Device imu(RAM::Control::IMU::AlgorithmType::COMPLEMENTARY,
                              1000.0 / SAMPLE_PERIOD_MS,
                              1000.0 / SAMPLE_PERIOD_MS,
                              mpuOutput + 3,
                              mpuOutput,
                              nullptr,
                              angles.m_objects);


const float U_THROTTLE_MIN = 0.;
const float U_THROTTLE_MAX = 1.;
//
// note that this matix is a function of ControlActions (throttle first)
constexpr float mixerFactor = 0.025;//0.25;
const float mixer[16] =
{
    1,  mixerFactor,  mixerFactor, -mixerFactor,
    1, -mixerFactor, -mixerFactor, -mixerFactor,
    1, -mixerFactor,  mixerFactor,  mixerFactor,
    1,  mixerFactor, -mixerFactor,  mixerFactor
};
arm_matrix_instance_f32 mixerMatrix, umotorVector, uactionVector;
bool isCtrlOn;

//
// application command to read out angle and filtered angle
//
enum SerialCommandId
{
    READOUT = 0,
    PIDPARAM
};

struct ReadoutCmd:
    public Command
{
    ReadoutCmd():
        Command(
            ReceiverId::APPLICATION,
            READOUT)
    {;}
    
    void execute()
    {
        float data[10];

        reinterpret_cast<uint32_t *>(data)[0] = RAM::Target::SystemTick::jiffies();
        
        data[1] = references[DroneVariables::PITCH];      // g

        data[2] = angles[DroneVariables::PITCH];          // b

        data[3] = gyro[DroneVariables::PITCH];            // y

        data[4] = uattitude[DroneVariables::THROTTLE];    // y

        data[5] = uattitude[DroneVariables::PITCH];       // g

        memcpy(data + 6, umotor.m_objects, umotor.m_list.size() * sizeof(float));
        
        reply(
            ReplyId::OK,
            reinterpret_cast<uint8_t *>(data),
            sizeof(data));
    }
};

struct PidCmd:
    public Command
{
    PidCmd():
        Command(
            ReceiverId::APPLICATION,
            PIDPARAM)
    {;}
    
    void execute()
    {
        uint8_t controller;
        float params[4];

        if ( !read(&controller, sizeof(controller)) || controller > pids.m_list.size() )
        {
            reply(ReplyId::UNKNOWN);
            
            return;
        }

        if ( !read(params, sizeof(params)) )
        {
            reply(ReplyId::UNKNOWN);
            
            return;
        }

        pids[static_cast<DroneVariables>(controller)].setParams(params[0], params[1], params[2]);
        
        reply(ReplyId::OK);
    }
};

ReadoutCmd readoutCmd;
PidCmd pidCmd;

void setThrottle(const float dutyCycle)
{
    pwm0.setDutyCycle(RAM::Target::PWM::SelectId::PWMA, dutyCycle);
    
    pwm0.setDutyCycle(RAM::Target::PWM::SelectId::PWMB, dutyCycle);
    
    pwm1.setDutyCycle(RAM::Target::PWM::SelectId::PWMA, dutyCycle);

    pwm1.setDutyCycle(RAM::Target::PWM::SelectId::PWMB, dutyCycle);
}

void setupPids()
{
    arm_mat_init_f32(&mixerMatrix, 4, 4, (float32_t *)mixer);

    arm_mat_init_f32(&uactionVector, 4, 1, uattitude.m_objects);

    arm_mat_init_f32(&umotorVector, 4, 1, umotor.m_objects);

    pids[DroneVariables::ROLL].setParams(1., 0.1, 0.);

    pids[DroneVariables::PITCH].setParams(0.1, 0.001, 0.1);

    pids[DroneVariables::YAW].setParams(1., 0.1, 0.);
}

void setupRc()
{
    unsigned cnt = 0, frame, resync;

    //
    // 1, motors pwm were initialized at maximum duty cycle
    // 2. wait for first rc reading; this could take as long as the
    //    frame period, during which time the ESC should have switched
    //    to calibration mode due to #1 above (a very safe mode than
    //    propellers spinning at high speed),
    // 3. if rc reading not available then zeroed all ESCs and bail out
    // 4. apply throttle reading to motors
    // 5. if throttle high then calibrate ESCs
    // 6. wait until CALIBWAIT channel cleared
    // 7. read RC zeros
    // 8. read attitude/rate control mode channel
    // 9. hardcode channels invertion flags
    //

    do
    {
        RAM::Support::Utils::delayus(1000);
        
        ppm->readout(frame, resync, rcTimings.m_objects);
        
    } while ( frame == 0 && ++cnt < 400 );

    println(RAM::Target::SystemTick::micros());
        
    if ( cnt == 400 )
    {
        setThrottle(0.);

        RAM::Support::Error::longjmp(TURNIGY_HAL_ERR_RCNOK);
    }
    else
    {
        setThrottle(rcTimings[DroneVariables::THROTTLE]);
    }

    //
    // if throttle bigger than 90% then calibrate ESC
    //
    if ( rcTimings[DroneVariables::THROTTLE] > 0.9 )
    {
        while ( rcTimings[DroneVariables::THROTTLE] > 0.9 )
        {
            ppm->readout(frame, resync, rcTimings.m_objects);
        }
        
        //
        // iterate now for a couple of seconds updating motors with
        // the current throttle reading
        //
        for ( unsigned i = 0; i < 1000; ++i )
        {
            ppm->readout(frame, resync, rcTimings.m_objects);
            
            setThrottle(rcTimings[DroneVariables::THROTTLE]);
            
            RAM::Support::Utils::delayus(1000);
        }
    }
    
    //
    // wait until CALIBWAIT channel is cleared
    //
    while ( rcTimings[DroneVariables::CALIBWAIT] > 0.5 )
    {
        ppm->readout(frame, resync, rcTimings.m_objects);
        
        mpu6050.calibrate();
    }

    //
    // remember rc references at this moment, they represent the
    // zero reference for all calculations after this point in time.
    //
    for ( const auto &i : rcTimings0.m_list )
    {
        rcTimings0[i] = rcTimings[i];
    }
    
    //
    // initialize controller on/off flag
    //
    if ( rcTimings[DroneVariables::CTRLTYPE] > 0.5 )
    {
        isCtrlOn = true;
    }
    else
    {
        isCtrlOn = false;
    }

    print("attitude controller is ");
    println(isCtrlOn);

    rcInverted[DroneVariables::ROLL] = false;
     
    rcInverted[DroneVariables::PITCH] = false;
     
    rcInverted[DroneVariables::THROTTLE] = true;
     
    rcInverted[DroneVariables::YAW] = false;
}

/** Read RC timings and convert to attitude reference values in process units
 ** (radians)
 **
 ** Angular conversion in attitude control mode:
 **
 **     rc  0         1
 **        -+---------+-
 **         |         |
 **  roll  -pi/2     +pi/2
 **  pitch -pi/2     +pi/2
 **  yaw   -pi       +pi (ground reference respect north)    
 **
 */
void rcToReference(
    ControlActions<float> &refs,
    RemoteControlChannels<float> &rcs,
    AttitudeAxes<float> &rcs0,
    RemoteControlChannels<bool> &rcInverted)
{
    refs[DroneVariables::ROLL] =
        MPI
        *
        (rcs[DroneVariables::ROLL] - rcs0[DroneVariables::ROLL]);

    refs[DroneVariables::PITCH] =
        MPI
        *
        (rcs[DroneVariables::PITCH] - rcs0[DroneVariables::PITCH]);

    //
    // in this case the yaw reference varies between -pi and pi.
    //
    refs[DroneVariables::YAW] =
        2 * MPI
        *
        (rcs[DroneVariables::YAW] - rcs0[DroneVariables::YAW]);

    for ( auto i : {DroneVariables::ROLL, DroneVariables::PITCH, DroneVariables::YAW} )
    {
        if ( rcInverted[i] )
        {
            refs[i] *= -1;
        }
    }
    
    refs[DroneVariables::THROTTLE] = rcs[DroneVariables::THROTTLE];
}

/** Read RC control settings and update attitude PID actions.
 */ 
void updateAttitudeAction()
{
    //
    // if not enough throttle action then reset controllers and keep
    // angles actions zeroed
    //
    if ( rcTimings[DroneVariables::THROTTLE] == 0 )
    {
        uattitude[DroneVariables::ROLL] = 0.;
        
        uattitude[DroneVariables::PITCH] = 0.;

        uattitude[DroneVariables::YAW] = 0.;

        for ( unsigned i = 0; i < pids.m_list.size(); ++i )
        {
            pids.m_objects[i].reset();
        }

        return;
    }

    //
    // if controller on/off has changed then take action
    //
    if ( (rcTimings[DroneVariables::CTRLTYPE] > 0.5) != isCtrlOn )
    {
        isCtrlOn = !isCtrlOn;

        //
        // if just restarting control then reset pid controller
        //
        if ( isCtrlOn )
        {
            for ( unsigned i = 0; i < pids.m_list.size(); ++i )
            {
                pids.m_objects[i].reset();
            }
        }

        print("attitude controller is ");
        println(isCtrlOn);
    }

    //
    // throttle action always translates one-to-one
    //
    uattitude[DroneVariables::THROTTLE] = rcTimings[DroneVariables::THROTTLE];

    if ( isCtrlOn )
    {
        //
        // range of action angles is enforced to [-pi/2, pi/2]. Note that
        // rc timings range in [0,1], and if sticks were at rest during
        // initialization then roll0 and pitch0 are equal ~0.5.
        //
        uattitude[DroneVariables::ROLL] = 0;//arm_pid_f32(&pid[PID_ROLL], MPI * (rcTimings[RC_ROLL] - roll0) - angles[M_ROLL]);
        uattitude[DroneVariables::PITCH] =
            pids[DroneVariables::PITCH].update(
                references[DroneVariables::PITCH] - angles[DroneVariables::PITCH],
                U_THROTTLE_MIN - uattitude[DroneVariables::THROTTLE],
                U_THROTTLE_MAX - uattitude[DroneVariables::THROTTLE]);
//        uattitude[U_PITCH] =
//            controllers[PID_PITCH].process(
//                MPI * (rcTimings[RC_PITCH] - pitch0),
//                angles[IMU_PITCH],
//                );
            
        //
        // yaw is different, it is a rate based axis
        // FIXME: missing proportional conversion factor.
        //
        uattitude[DroneVariables::YAW] = 0;//rcTimings[RC_YAW] - yaw0;

    }
    else
    {
        //
        // just copy rc commands to attitude actions.
        //
        for ( const auto &i : uattitude.m_list )
        {
            uattitude[i] = references[i];
        }
    }
}

/** Translate attitude actions into motor actions and apply (PWM).
 */
void updateMotorAction()
{
    //
    // from action to motor signal by means of mixer matrix
    //
    arm_mat_mult_f32(&mixerMatrix, &uactionVector, &umotorVector);
    
    //
    // saturate motor signals
    //
    for ( unsigned int i = 0; i < umotor.m_list.size(); ++i )
    {
        if ( umotor.m_objects[i] > 1.0 )
        {
            umotor.m_objects[i] = 1.0;
        }
        else if ( umotor.m_objects[i] < 0 )
        {
            umotor.m_objects[i] = 0;
        }
    }

    pwm0.setDutyCycle(
        RAM::Target::PWM::SelectId::PWMA,
        umotor[DroneVariables::U3]);
 
    pwm0.setDutyCycle(
        RAM::Target::PWM::SelectId::PWMB,
        umotor[DroneVariables::U4]);
 
    pwm1.setDutyCycle(
        RAM::Target::PWM::SelectId::PWMA,
        umotor[DroneVariables::U1]);
 
    pwm1.setDutyCycle(
        RAM::Target::PWM::SelectId::PWMB,
        umotor[DroneVariables::U2]);
}

void setup()
{
    RAM::Logic::I2C::instance().setup(400, false);

    RAM::Logic::Host::CommandReply::CommandList::instance().
        setCommandHandler(&readoutCmd);

    RAM::Logic::Host::CommandReply::CommandList::instance().
        setCommandHandler(&pidCmd);

    ppm = new PPM::Decoder<RAM::Target::Timer::ModuleId::TIMID1>(7, 18000, 2070);
    
    setupRc();

    setupPids();
    
    mpu6050.initialize();

    mpu6050.calibrate();
}

void algorithm()
{
    unsigned frame, resync;
    static unsigned counter = 0;

    mpu6050.readout();

    imu.update();

    //
    // read controller references
    //
    ppm->readout(frame, resync, rcTimings.m_objects);

    //
    // convert rc timings to references in process units (radians or
    // radians per second)
    //
    rcToReference(references, rcTimings, rcTimings0, rcInverted);

    updateAttitudeAction();
    
    updateMotorAction();
    
//    if ( rcTimings[RC_LOGGER] > 0.5 )
//    {
//        println("----");
//        println(counter);
//        println(angles[IMU_PITCH]);
//        println(MPI * (rcTimings[RC_PITCH] - pitch0) - angles[IMU_PITCH]);
//        println(uattitude[U_PITCH]);
//        println(umotor[U_2] + umotor[U_4] - (umotor[U_1] + umotor[U_3]));
//        println(rcTimings[RC_RATE_CTRL]);
//    }
    
    ++counter;
}

void RAM::Target::Main::setup()
{
    RAM::Logic::MainLoop::Executive::instance().
        setup(
            ::setup,
            ::algorithm,
            RAM::Logic::MainLoop::SerialChannelSelector::SAME_SERIAL_CHANNELS,
            SAMPLE_PERIOD_MS * 1000);
}

void RAM::Target::Main::loop()
{
    RAM::Logic::MainLoop::Executive::instance().execute();
}

/* ___oOo___ */
