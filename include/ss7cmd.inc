// =============================================================================
//   (    (                   *     (      
//   )\ ) )\ )     )   (    (  `    )\ )   
//  (()/((()/(  ( /(   )\   )\))(  (()/(   
//   /(_))/(_)) )\())(((_) ((_)()\  /(_))  
//  (_)) (_))  ((_)\ )\___ (_()((_)(_))_   
//  / __|/ __||__  /((/ __||  \/  | |   \  
//  \__ \\__ \  / /  | (__ | |\/| | | |) | 
//  |___/|___/ /_/    \___||_|  |_| |___/  
//                                        
//
//   ss7cmd v1.0.0-TITAN — Master Include
//   Modern Command Framework for open.mp/SA-MP
//
//   USAGE:
//     #include <open.mp> / <sa-mp>
//     #include <ss7cmd>
//
// =============================================================================

// =============================================================================
//   ss7cmd.inc — TITAN Master Include (Command + Database)
// =============================================================================

#if defined _ss7cmd_included
    #endinput
#endif
#define _ss7cmd_included

// =============================================================================
//  MODULE LOAD ORDER
// =============================================================================

#include "ss7_core.inc"      // 
#include "ss7_hook.inc"      // 
#include "ss7_parser.inc"    // 
#include "ss7_ui.inc"        // 
#include "ss7_registry.inc"  // 
#include "ss7_runtime.inc"   // 

// =============================================================================
//  VERSION INFORMATION
// =============================================================================

#define SS7_VERSION         "1.0.0-TITAN"
#define SS7_CODENAME        "TITAN"

// =============================================================================
//  COMPILE-TIME VERIFICATION
// =============================================================================

#if !defined _ss7_core_included
    #error "[ss7cmd] ss7_core.inc not loaded"
#endif
#if !defined _ss7_hook_included
    #error "[ss7cmd] ss7_hook.inc not loaded"
#endif
#if !defined _ss7_parser_included
    #error "[ss7cmd] ss7_parser.inc not loaded"
#endif
#if !defined _ss7_ui_included
    #error "[ss7cmd] ss7_ui.inc not loaded"
#endif
#if !defined _ss7_registry_included
    #error "[ss7cmd] ss7_registry.inc not loaded"
#endif

// =============================================================================
//  END OF ss7cmd.inc
// =============================================================================