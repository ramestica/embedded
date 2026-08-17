//
// Target stuff
//
#include <Target/I2C.h>

//
// Local stuff
//
#include <Logic/I2C.h>

using namespace RAM::Logic::Host::CommandReply;
namespace Target = RAM::Target::I2C;

//
// FIXME: hardcoded to I2CID0, the id should actually be a parameter to the I2C
// class.
//
Target::Module module;

//-------------------------------------------------------------------------------
RAM::Logic::I2C::I2C():
    m_kbps(0),
    m_enableInternalPullup(false),
    m_errorsCount(0),
    m_pingCmd(this),
    m_readCmd(this),
    m_writeCmd(this),
    m_setBitMaskCmd(this),
    m_unsetBitMaskCmd(this)
{
    CommandList::instance().setCommandHandler(&m_pingCmd);
    
    CommandList::instance().setCommandHandler(&m_readCmd);

    CommandList::instance().setCommandHandler(&m_writeCmd);

    CommandList::instance().setCommandHandler(&m_setBitMaskCmd);

    CommandList::instance().setCommandHandler(&m_unsetBitMaskCmd);
}

//-------------------------------------------------------------------------------
void RAM::Logic::I2C::setup(
    const uint16_t kbps,
    const bool enableInternalPullup)
{
    int err;
    
    if ( (err = ::Target::configure(
              ::Target::ModuleId::I2CID0,
              kbps,
              enableInternalPullup,
              module)) )
    {
        RAM::Support::Error::longjmp(err);
    }

    if ( (m_dataStream_p = RAM::Logic::Host::getCommandReplyDataStream())
         == nullptr )
    {
        RAM::Support::Error::longjmp(SERIAL_COMMANDS_ERR_DATA_STREAM_NULL);
    }
}

//-------------------------------------------------------------------------------
void RAM::Logic::I2C::write(uint8_t addr, uint8_t reg, uint8_t val)
{
    ::Target::write(module, true, addr, &reg, 1);

    ::Target::write(module, false, addr, &val, 1);
    
    ::Target::stop(module);
}

//-------------------------------------------------------------------------------
void RAM::Logic::I2C::write(
    const uint8_t addr,
    const uint8_t reg,
    const uint8_t mask,
    const uint8_t val)
{
    uint8_t tmp;

    ::Target::write(module, true, addr, &reg, 1);

    ::Target::read(module, true, addr, &tmp, 1);

    ::Target::write(module, true, addr, &reg, 1);

    tmp = tmp | (val & mask);

    ::Target::write(module, false, addr, &tmp, 1);

    ::Target::stop(module);
}

//-------------------------------------------------------------------------------
bool RAM::Logic::I2C::write(
    const uint8_t addr,
    const uint8_t reg,
    const void *buf,
    const size_t size)
{
    return ::Target::write(module, addr, reg, buf, size) == size;
}

//-------------------------------------------------------------------------------
void RAM::Logic::I2C::bitset(
    const uint8_t addr,
    const uint8_t reg,
    const uint8_t val)
{
    uint8_t tmp;

    ::Target::write(module, true, addr, &reg, 1);

    ::Target::read(module, true, addr, &tmp, 1);

    ::Target::write(module, true, addr, &reg, 1);

    tmp = tmp | val;

    ::Target::write(module, false, addr, &tmp, 1);

    ::Target::stop(module);
}

//-------------------------------------------------------------------------------
void RAM::Logic::I2C::bitunset(
    const uint8_t addr,
    const uint8_t reg,
    const uint8_t val)
{
    uint8_t tmp;

    ::Target::write(module, true, addr, &reg, 1);

    ::Target::read(module, true, addr, &tmp, 1);

    ::Target::write(module, true, addr, &reg, 1);

    tmp = tmp & ~val;

    ::Target::write(module, false, addr, &tmp, 1);

    ::Target::stop(module);
}

//-------------------------------------------------------------------------------
uint8_t RAM::Logic::I2C::read(uint8_t addr, uint8_t reg)
{
    uint8_t tmp;

    ::Target::write(module, true, addr, &reg, 1);

    ::Target::read(module, true, addr, &tmp, 1);
  
    ::Target::stop(module);

    return tmp;
}
        
//-------------------------------------------------------------------------------
bool RAM::Logic::I2C::read(
    const uint8_t addr,
    const uint8_t reg,
    void *buf,
    const size_t size)
{
    return ::Target::read(module, addr, reg, buf, size) == size;
}
        
//-------------------------------------------------------------------------------
bool RAM::Logic::I2C::ping(uint8_t addr)
{
    return ::Target::ping(module, addr);
}
        
//-------------------------------------------------------------------------------
bool RAM::Logic::I2C::serialReadAddr(Command &cmd, uint8_t &addr)
{
    char tmp;

    if ( m_dataStream_p->read(tmp) == 0 )
    {
        cmd.reply(ReplyId::MISSING_ADDR_PARAM);

        return false;
    }

    addr = tmp;

    return true;
}

//-------------------------------------------------------------------------------
bool RAM::Logic::I2C::serialReadAddrReg(Command &cmd, uint8_t &addr, uint8_t &reg)
{
    char tmp;

    if ( !serialReadAddr(cmd, addr) )
    {
        return false;
    }

    if ( m_dataStream_p->read(tmp) == 0 )
    {
        cmd.reply(ReplyId::MISSING_REG_PARAM);

        return false;
    }

    reg = tmp;

    return true;
}

//-------------------------------------------------------------------------------
bool RAM::Logic::I2C::serialReadAddrRegVal(
    Command &cmd,
    uint8_t &addr,
    uint8_t &reg,
    uint8_t &val)
{
    if ( !serialReadAddrReg(cmd, addr, reg) )
    {
        return false;
    }

    if ( m_dataStream_p->read(reinterpret_cast<char &>(val)) == 0 )
    {
        cmd.reply(ReplyId::MISSING_VALUE_PARAM);

        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------
bool RAM::Logic::I2C::serialReadAddrRegLen(
    Command &cmd,
    uint8_t &addr,
    uint8_t &reg,
    uint8_t &length)
{
    char tmp;

    if ( !serialReadAddrReg(cmd, addr, reg) )
    {
        return false;
    }

    if ( m_dataStream_p->read(tmp) == 0 )
    {
        cmd.reply(ReplyId::MISSING_LENGTH_PARAM);

        return false;
    }

    length = tmp;

    return true;
}

//-------------------------------------------------------------------------------
void RAM::Logic::I2C::PingCmd::execute()
{
    char tmp;
    uint8_t status = 0;
    uint8_t addr;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_ADDR_PARAM);

        return;
    }

    addr = tmp;

    if ( m_obj_p->ping(addr) )
    {
        status = 1;
    }

    reply(ReplyId::OK, &status, sizeof(status));
}

//-------------------------------------------------------------------------------
void RAM::Logic::I2C::ReadCmd::execute()
{
    char tmp;
    uint8_t addr, reg, length;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_ADDR_PARAM);

        return;
    }

    addr = tmp;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_REG_PARAM);

        return;
    }

    reg = tmp;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_LENGTH_PARAM);

        return;
    }

    length = tmp;

    if ( length > 256 )
    {
        reply(SerialReplyId::LENGTH_HUGE);

        return;
    }

    uint8_t data[length];

    if ( !m_obj_p->read(addr, reg, data, length) )
    {
        reply(SerialReplyId::LENGTH_MISMATCH);

        return;
    }

    reply(ReplyId::OK, data, length);
}

//-------------------------------------------------------------------------------
void RAM::Logic::I2C::WriteCmd::execute()
{
    char tmp;
    uint8_t addr, reg, value;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_ADDR_PARAM);

        return;
    }

    addr = tmp;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_REG_PARAM);

        return;
    }

    reg = tmp;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_VALUE_PARAM);

        return;
    }

    value = tmp;

    if ( !m_obj_p->write(addr, reg, &value, 1) )
    {
        reply(SerialReplyId::LENGTH_MISMATCH);

        return;
    }

    reply(ReplyId::OK);
}

//-------------------------------------------------------------------------------
void RAM::Logic::I2C::SetBitMaskCmd::execute()
{
    char tmp;
    uint8_t addr, reg, value;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_ADDR_PARAM);

        return;
    }

    addr = tmp;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_REG_PARAM);

        return;
    }

    reg = tmp;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_VALUE_PARAM);

        return;
    }

    value = tmp;

    m_obj_p->bitset(addr, reg, value);

    reply(ReplyId::OK);
}

//-------------------------------------------------------------------------------
void RAM::Logic::I2C::UnsetBitMaskCmd::execute()
{
    char tmp;
    uint8_t addr, reg, value;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_ADDR_PARAM);

        return;
    }

    addr = tmp;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_REG_PARAM);

        return;
    }

    reg = tmp;

    if ( m_obj_p->m_dataStream_p->read(tmp) == 0 )
    {
        reply(ReplyId::MISSING_VALUE_PARAM);

        return;
    }

    value = tmp;

    m_obj_p->bitunset(addr, reg, value);

    reply(ReplyId::OK);
}

/*___oOo___*/
