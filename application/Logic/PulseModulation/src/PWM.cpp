/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Base stuff
//

//
// Target stuff
//
 
//
// Library stuff
//
#include <ErrorCodes.h>

//
// Local stuff
//
#include "PulseModulation.h"

using namespace RAM::Logic::PulseModulation;

#define DUTYCYCLE(dc)                                                   \
    (((m_dc100us - m_dc0us) * dc + m_dc0us) / float(m_generator.m_periodus))

//-------------------------------------------------------------------------------
PWM::Encoder::Encoder(
    const RAM::Target::PWM::GeneratorId gid,
    const RAM::Target::PWM::SelectId sid,
    const uint32_t periodus,
    const uint32_t dc0us,
    const uint32_t dc100us,
    const float initialDutyCycle):
    m_setDutyCycleCmd(gid, this)
{
    RAM::Logic::Host::CommandReply::CommandList::instance().setCommandHandler(&m_setDutyCycleCmd);

    m_dc0us = dc0us;

    m_dc100us = dc100us;

    if ( RAM::Target::PWM::configure(gid, sid, periodus, m_generator) )
    {
        RAM::Support::Error::longjmp(PWM_ERR_CONFIG_FAILED);
    }

    if ( m_dc0us >= m_dc100us || m_dc100us > periodus )
    {
        RAM::Support::Error::longjmp(PWM_ERR_INVALID_DC_PARAMS);
    }

    //
    // note that's important to use DUTYCYCLE only after all other member
    // variables had been initialized already
    //
    float dc[2] = {DUTYCYCLE(initialDutyCycle), DUTYCYCLE(initialDutyCycle)};
    
    if ( RAM::Target::PWM::start(m_generator, dc, nullptr) )
    {
        RAM::Support::Error::longjmp(PWM_ERR_START_FAILED);
    }
}

//-------------------------------------------------------------------------------
void PWM::Encoder::setDutyCycle(
    const RAM::Target::PWM::SelectId sid,
    const float &dc)
{
    if ( RAM::Target::PWM::setDutyCycle(
             m_generator,
             sid,
             DUTYCYCLE(dc)) )
    {
        RAM::Support::Error::longjmp(PWM_ERR_SET_DUTY_CYCLE);
    }
}

//-------------------------------------------------------------------------------
PWM::Encoder::SetDutyCycleCmd::SetDutyCycleCmd(
    const RAM::Target::PWM::GeneratorId gid,
    PWM::Encoder *obj):
    Command(
        (gid == RAM::Target::PWM::GeneratorId::PWMGENID0)
        ?
        RAM::Logic::Host::CommandReply::ReceiverId::PWMENC0
        :
        (gid == RAM::Target::PWM::GeneratorId::PWMGENID1)
        ?
        RAM::Logic::Host::CommandReply::ReceiverId::PWMENC1
        :
        (gid == RAM::Target::PWM::GeneratorId::PWMGENID2)
        ?
        RAM::Logic::Host::CommandReply::ReceiverId::PWMENC2
        :
        (gid == RAM::Target::PWM::GeneratorId::PWMGENID3)
        ?
        RAM::Logic::Host::CommandReply::ReceiverId::PWMENC3
        :
        (gid == RAM::Target::PWM::GeneratorId::PWMGENID4)
        ?
        RAM::Logic::Host::CommandReply::ReceiverId::PWMENC4
        :
        (gid == RAM::Target::PWM::GeneratorId::PWMGENID5)
        ?
        RAM::Logic::Host::CommandReply::ReceiverId::PWMENC5
        :
        (gid == RAM::Target::PWM::GeneratorId::PWMGENID6)
        ?
        RAM::Logic::Host::CommandReply::ReceiverId::PWMENC6
        :
        RAM::Logic::Host::CommandReply::ReceiverId::PWMENC7,
        int(SerialCommandId::SETDUTY)),
    m_obj_p(obj)
{
}

//-------------------------------------------------------------------------------
void PWM::Encoder::SetDutyCycleCmd::execute()
{
    uint8_t sid;
    float dc;

    //
    // if read fails to read the parameter then itself has sent an error reply
    //
    if ( !read(&sid, sizeof(sid)) )
    {
        return;
    }

    //
    // if read fails to read the parameter then itself has sent an error reply
    //
    if ( !read(&dc, sizeof(dc)) )
    {
        return;
    }

    m_obj_p->setDutyCycle(
        (sid == 0)
        ?
        RAM::Target::PWM::SelectId::PWMA
        :
        (sid == 1)
        ?
        RAM::Target::PWM::SelectId::PWMB
        :
        RAM::Target::PWM::SelectId::PWMBOTH,
        dc);

    reply(RAM::Logic::Host::CommandReply::ReplyId::OK);
}

/*___oOo___*/
