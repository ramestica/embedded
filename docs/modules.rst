Logging Module with Host and Target Variants
============================================

That’s a good example, and it’s exactly where you want to be careful
not to encode “flavors” as separate modules with the same conceptual
identity but different names like host_logging vs target_logging
unless you truly intend them to diverge long-term.

What you actually have is:

one logical module (logging) with multiple build variants /
implementations

not two independent modules.
This is a common situation in embedded or cross-platform systems:

You have one logical module (``logging``), but multiple implementations depending on the environment:

- Host build (runs on development machine)
- Target build (runs on embedded system)

The key design rule is:

**Do not model these as separate modules unless they are truly different systems.**

Instead, treat them as:

- One module identity
- Multiple implementations or build variants

---

Module Identity
---------------

There should be a single module:

.. code-block:: cmake

    embedded_module(NAME logging)

This defines a single conceptual component: ``logging``.

---

Option 1: Single Target, Conditional Backend
--------------------------------------------

Use a configuration option to select the implementation:

.. code-block:: cmake

    option(LOGGING_BACKEND "logging backend (host|target)" "host")

    if(LOGGING_BACKEND STREQUAL "host")
        add_library(logging src/logging_host.cpp)

    elseif(LOGGING_BACKEND STREQUAL "target")
        add_library(logging src/logging_target.cpp)
    endif()

    target_include_directories(logging PUBLIC include)
    add_library(Embedded::logging ALIAS logging)

This approach keeps:

- One module
- One public API
- Different implementations

---

Option 2: Two Internal Targets, One Module
------------------------------------------

If both implementations must exist at the same time:

.. code-block:: cmake

    add_library(logging_host STATIC src/logging_host.cpp)
    add_library(logging_target STATIC src/logging_target.cpp)

    target_include_directories(logging_host PUBLIC include)
    target_include_directories(logging_target PUBLIC include)

    add_library(Embedded::logging_host ALIAS logging_host)
    add_library(Embedded::logging_target ALIAS logging_target)

Then select implementation at a higher level:

.. code-block:: cmake

    if(EMBED_ON_HOST)
        set(LOGGING_IMPL Embedded::logging_host)
    else()
        set(LOGGING_IMPL Embedded::logging_target)
    endif()

    target_link_libraries(app PRIVATE ${LOGGING_IMPL})

---

Option 3: Interface + Implementations (Recommended)
---------------------------------------------------

This is the most scalable architecture.

Define a common interface:

.. code-block:: cmake

    add_library(logging_interface INTERFACE)

    target_include_directories(logging_interface INTERFACE include)

Provide implementations:

.. code-block:: cmake

    add_library(logging_host STATIC src/logging_host.cpp)
    add_library(logging_target STATIC src/logging_target.cpp)

    target_link_libraries(logging_host PUBLIC logging_interface)
    target_link_libraries(logging_target PUBLIC logging_interface)

All consumers depend only on:

.. code-block:: cmake

    Embedded::logging_interface

Selection happens at a higher level.

---

What NOT to Do
--------------

Avoid splitting into separate modules like:

.. code-block:: text

    logging_host/
    logging_target/

if:

- They share the same API
- They differ only by platform/backend
- They evolve together

This leads to:

- Duplicated interfaces
- Diverging APIs over time
- Harder dependency management

---

When Splitting Into Separate Modules Is Valid
---------------------------------------------

Separate modules only make sense if:

- APIs differ significantly
- Lifecycles differ (tool vs firmware)
- Dependencies differ strongly
- They are independently versioned

Example:

.. code-block:: text

    modules/
        logging_host/
        logging_firmware/

Each becomes:

.. code-block:: cmake

    embedded_module(NAME logging_host)
    embedded_module(NAME logging_firmware)

---

Mental Model
------------

Think in terms of:

**Module = identity (stable concept)**  
**Targets = implementations (variable per platform/config)**

So in your case:

- ``logging`` → module identity
- host/target → implementation variants

not separate modules.

---

If desired, this structure can be generalized into a full ``embedded_module()`` design that supports:

- host vs target toolchains
- multiple implementations per module
- mixed-language modules (C/C++/Python/scripts)
- clean dependency management without CMake complexity explosion

Module Layering
===============

The module layering, from bottom to top.

The contract (``devices/i2c/``)
-------------------------------

A header-only interface, ``i2c_hal.h``, defining the abstract I2C
transaction primitives (``write``, ``write_read`` for repeated-start) and a
status enum. It is a CMake ``INTERFACE`` library (``i2c``), depends on
nothing, and knows about neither vendors nor slaves. This is the seam the
whole design turns on.

The implementations (``devices/hal/``)
--------------------------------------

One backend per platform, each providing the same ``i2c-hal`` target by
fulfilling the contract:

- ``tm4c123/`` — wraps TivaWare driverlib
- ``stm32f1/`` — wraps ST LL
- ``linux/`` — wraps ``/dev/i2c`` or a USB-I2C bridge

Which one compiles is a **configure-time choice** via
``-DI2C_HAL=<backend>``, dispatched in ``devices/hal/CMakeLists.txt``. Each
links the contract (``i2c``) but never the protocol above it — the HAL does
not know the FPGA exists.

The protocol (``control/mac/``)
-------------------------------

``fpga_cmd.c``, the command/reply logic for the Spartan-6 slave: the
``OK`` / ``ERR`` / ``NOREPLY`` trichotomy, timeouts, retries, idempotency. It
depends *only* on the contract, so it is **freestanding and dual-target** —
the exact same source compiles for Cortex-M firmware and for the host CLI.
That portability is only possible because it talks to ``i2c_hal.h``, never
to a specific peripheral.

The applications (leaves)
-------------------------

Firmware apps (``tm4c123/fpga-ctrl/``, etc.) and the host CLI
(``tools/macctl/``) each link ``mac`` + ``i2c-hal``, supplying a concrete
backend to satisfy the protocol layer's abstract dependency. The dependency
graph is ``mac → i2c ← i2c-hal``, joined only in the final executable.

Holding principles
------------------

- **Dependencies point downward toward the abstract contract.** Protocol and
  HAL both depend on ``i2c``, neither on each other — so a new slave module
  or a new MCU slots in without touching anything else.
- **The contract stays minimal.** Only transaction primitives cross it; init
  (clock/pin setup) stays platform-specific and is called directly from each
  app's ``main()``, never abstracted.

Payoffs
-------

- Because the CLI talks to both the FPGA and MCU directly, it is just another
  HAL backend (``linux``) rather than special-cased code.
- Because the protocol layer depends only on the contract, a ``mock`` HAL
  makes ``control/mac`` unit-testable on the host under ``ctest``, pinning
  down the retry and ``NOREPLY`` edge cases with no hardware.
  
