#ifndef _Main_H_
#define _Main_H_

//
// System stuff
//

//
// Target stuff
//

//
// RAM stuff
// 

//
// Local stuff
//

namespace RAM
{
    namespace Target
    {
        namespace Main
        {
            /** Setup application. Implemented by application code.
             */
            extern void setup();

            /** Execute application iteration. Implemented by application code.
             */
            extern void loop();
        }
    }
}

#endif /* _Main_H_ */

/*___oOo__*/
