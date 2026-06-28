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
