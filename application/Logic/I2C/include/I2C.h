#ifndef _I2C_H_
#define _I2C_H_

//
// System stuff
//

//
// Target stuff
//

//
// Library stuff
//
#include <ErrorCodes.h>
#include <Host.h>

namespace RAM
{
    namespace Logic
    {
        class I2C
        {
        public:
            static I2C &instance()
            {
                static I2C instance;

                return instance;
            }

            void setup(
                const uint16_t kbps,
                const bool enableInternalPullup);

            void write(
                uint8_t addr,
                uint8_t reg,
                uint8_t val);

            void write(
                const uint8_t addr,
                const uint8_t reg,
                const uint8_t mask,
                const uint8_t val);

            void bitset(
                const uint8_t addr,
                const uint8_t reg,
                const uint8_t val);

            void bitunset(
                const uint8_t addr,
                const uint8_t reg,
                const uint8_t val);

            uint8_t read(
                uint8_t addr,
                uint8_t reg);
        
            bool read(
                const uint8_t addr,
                const uint8_t reg,
                void *buf,
                const size_t size);

            bool write(
                const uint8_t addr,
                const uint8_t reg,
                const void *buf,
                const size_t size);

            bool ping(uint8_t addr);

            bool serialReadAddr(
                RAM::Logic::Host::CommandReply::Command &cmd,
                uint8_t &addr);
                
            bool serialReadAddrReg(
                RAM::Logic::Host::CommandReply::Command &cmd,
                uint8_t &addr,
                uint8_t &reg);
                
            bool serialReadAddrRegVal(
                RAM::Logic::Host::CommandReply::Command &cmd,
                uint8_t &addr,
                uint8_t &reg,
                uint8_t &length);

            bool serialReadAddrRegLen(
                RAM::Logic::Host::CommandReply::Command &cmd,
                uint8_t &addr,
                uint8_t &reg,
                uint8_t &length);

        private:
            /** A private constructor makes a singleton.
             */
            I2C();

            RAM::Logic::Host::DataStream *m_dataStream_p;

            uint16_t m_kbps;

            bool m_enableInternalPullup;

            int16_t m_errorsCount;

            struct SerialCommandId
            {
                enum SerialCommandIdEnum
                {
                    PING = 0,
                    READ,
                    WRITE,
                    SET_BIT_MASK,
                    UNSET_BIT_MASK
                };
            };

            struct SerialReplyId
            {
                enum SerialReplyIdEnum
                {
                    LENGTH_MISMATCH = RAM::Logic::Host::CommandReply::ReplyId::OFFSET,
                    LENGTH_HUGE,
                    PING_NOACK
                };
            };

            class PingCmd: public RAM::Logic::Host::CommandReply::Command
            {
            public:
                PingCmd(I2C *ptr):
                    Command(RAM::Logic::Host::CommandReply::ReceiverId::I2C,
                            SerialCommandId::PING),
                    m_obj_p(ptr)
                {;}
                
                void execute();

                I2C *m_obj_p;
            };
            PingCmd m_pingCmd;
            
            class ReadCmd: public RAM::Logic::Host::CommandReply::Command
            {
            public:
                ReadCmd(I2C *ptr):
                    Command(RAM::Logic::Host::CommandReply::ReceiverId::I2C,
                            SerialCommandId::READ),
                    m_obj_p(ptr)
                {;}
                
                void execute();

                I2C *m_obj_p;
            };
            ReadCmd m_readCmd;
            
            class WriteCmd: public RAM::Logic::Host::CommandReply::Command
            {
            public:
                WriteCmd(I2C *ptr):
                    Command(RAM::Logic::Host::CommandReply::ReceiverId::I2C,
                            SerialCommandId::WRITE),
                    m_obj_p(ptr)
                {;}
                
                void execute();

                I2C *m_obj_p;
            };
            WriteCmd m_writeCmd;
            
            class SetBitMaskCmd: public RAM::Logic::Host::CommandReply::Command
            {
            public:
                SetBitMaskCmd(I2C *ptr):
                    Command(RAM::Logic::Host::CommandReply::ReceiverId::I2C,
                            SerialCommandId::SET_BIT_MASK),
                    m_obj_p(ptr)
                {;}
                
                void execute();

                I2C *m_obj_p;
            };
            SetBitMaskCmd m_setBitMaskCmd;
            
            class UnsetBitMaskCmd: public RAM::Logic::Host::CommandReply::Command
            {
            public:
                UnsetBitMaskCmd(I2C *ptr):
                    Command(
                        RAM::Logic::Host::CommandReply::ReceiverId::I2C,
                        SerialCommandId::UNSET_BIT_MASK),
                    m_obj_p(ptr)
                {;}
                
                void execute();

                I2C *m_obj_p;
            };
            UnsetBitMaskCmd m_unsetBitMaskCmd;
        };
    }
}

#endif /* _I2C_H_ */

/*___oOo__*/
