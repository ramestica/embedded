//
// Target stuff
//
#include <target/Target.h>
#include <Main.h>

#include <ErrorCodes.h>

//
// System stuff
//
#include <stdint.h>
#include <initializer_list>
		
//
// Local stuff
//

using namespace std;
using namespace RAM::Target;

#define SequentialEnum(Name,...)                                \
    enum Name { __VA_ARGS__ };                                  \
    namespace                                                   \
    {                                                           \
        const std::initializer_list<Name> Name##List { __VA_ARGS__ };   \
    };

SequentialEnum(Shape,
    Circle,
    Square,
    Triangle,
    Oval,
    Polygon
);

//-------------------------------------------------------------------------------
void RAM::Target::Main::setup()
{
    int cnt = 0;
    
    for ( Shape i : ShapeList )
    {
        if ( i == Shape::Circle )
        {
            break;
        }

        cnt++;
    }

    if ( cnt == 0 )
    {
        RAM::Support::Error::longjmp(MAINLOOP_ERR_SYSTICK);
    }
}

//-------------------------------------------------------------------------------
void RAM::Target::Main::loop()
{
}

/*___oOo___*/
