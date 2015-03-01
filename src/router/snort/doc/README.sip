SIP Preprocessor
================================================================================
Hui Cao

Overview
================================================================================
Session Initiation Protocol (SIP) is an application-layer control (signaling)
protocol for creating,   modifying, and terminating sessions with one or more 
participants.  These sessions include Internet telephone calls, multimedia 
distribution, and multimedia conferences. SIP Preprocessor provides ways to 
tackle Common Vulnerabilities and Exposures (CVEs) related with SIP found over 
the past few years. It also makes detecting new attacks easier.

Sections:
    Dependency Requirements
    Configuration
    Events
    Rule Options


Dependency Requirements
================================================================================
For proper functioning of the preprocessor:
 
    Stream session tracking must be enabled, i.e. stream5. Both TCP and UDP must be
      enabled in stream5. The preprocessor requires a session tracker to keep its 
      data. In addition, Stream API is able to provide correct support for ignoring 
      audio/video data channel.
    IP defragmentation should be enabled, i.e. the frag3 preprocessor should be
      enabled and configured.


Configuration
================================================================================
The preprocessor configuration name is "sip".

preprocessor sip

Option              Argument          Required   Default
disabled            None                  No     OFF
max_sessions        <max_sessions>        No     max_sessions 10000
max_dialogs         <max_dialogs>         No     max_dialogs  4
ports               <ports>               No     ports { 5060 5061 }
methods             <methods>             No     methods { invite cancel ack bye 
                                                       register options } 
max_uri_len         <max_uri_len>         No     max_uri_len 256
max_call_id_len     <max_call_id_len>     No     max_call_id_len 256
max_requestName_len <max_requestName_len> No     max_requestName_len 20
max_from_len        <max_from_len>        No     max_from_len 256
max_to_len          <max_to_len>          No     max_to_len 256
max_via_len         <max_via_len>         No     max_via_len 1024
max_contact_len     <max_contact_len>     No     max_contact_len 256
max_content_len     <max_content_len>     No     max_content_len 1024
ignore_call_channel None                  No     OFF

max_sessions        = 1024 - 4194303
max_dialogs         = 1 - 4194303
methods             = "invite" | "cancel" | "ack" | "bye" | "register" | "options"
                      | "refer" | "subscribe" | "update" | "join" | "info" | "message"                
                      | "notify" | "prack"
max_uri_len         = 0 - 65535
max_call_id_len     = 0 - 65535
max_requestName_len = 0 - 65535
max_from_len        = 0 - 65535
max_to_len          = 0 - 65535
max_via_len         = 0 - 65535
max_contact_len     = 0 - 65535
max_content_len     = 0 - 65535

Option explanations
   
  disabled
      SIP dynamic preprocessor can be enabled/disabled through configuration. 
      By default this value is turned off. When the preprocessor is disabled, 
      only the max_sessions option is applied when specified with the configuration.
  
  max_sessions
      This specifies the maximum number of sessions that can be allocated. 
      Those sessions are stream sessions, so they are bounded by maximum number of 
      stream sessions. Default is 10000.
  
  max_dialogs
      This specifies the maximum number of dialogs within one stream session. If exceeded,
      the oldest dialog will be dropped. Default is 4.
      
  ports
      This specifies on what ports to check for SIP messages. Typically, this will 
      include 5060, 5061.
      
      Syntax:
      ports { <port> [<port>< ... >] }
      
      Examples:
      ports { 5060 5061 }
      
      Note: there are spaces before and after '{' and '}'
      
  methods
      This specifies on what methods to check for SIP messages: (1) invite, 
      (2) cancel, (3) ack, (4) bye, (5) register, (6) options, (7) refer, 
      (8) subscribe, (9) update (10) join (11) info (12) message (13) notify 
      (14) prack
      Note: those 14 methods are up to date list (Feb. 2011). New methods can be 
      added to the list. Up to 32 methods supported.

      Syntax:
      methods { <method-list> }
      method-list = method|method method-list
      method      = "invite" | "cancel" | "ack" | "bye" | "register" | "options"
                    | "refer" | "subscribe" | "update" | "join" | "info" | "message"
                    | "notify"| "prack"
      Examples:
        methods { invite cancel ack bye register options }
      Add new method "information":
        methods { invite cancel ack bye register options information }
      
      Note: there are spaces before and after '{' and '}'
  
  max_uri_len
      This specifies the maximum Request_URI field size. If the Request_URI field
      is greater than this size, an alert is generated. Default is set to 256. 
      The allowed range for this option is 0 - 65535. "0" means never alert.
  
  max_call_id_len
      This specifies the maximum Call-ID field size. If the Call-ID field is 
      greater than this size, an alert is generated. Default is set to 256. 
      The allowed range for this option is 0 - 65535. "0" means never alert.
  
  max_requestName_len
      This specifies the maximum request name size that is part of the CSeq ID. 
      If the request name is greater than this size, an alert is generated. 
      Default is set to 20. The allowed range for this option is 0 - 65535. 
      "0" means never alert.
  
  max_from_len
      This specifies the maximum From field size. If the From field is greater 
      than this size, an alert is generated. Default is set to 256. The allowed
      range for this option is 0 - 65535. "0" means never alert.
  
  max_to_len
      This specifies the maximum To field size. If the To field is greater than
      this size, an alert is generated. Default is set to 256. The allowed range 
      for this option is 0 - 65535. "0" means never alert.
  
  max_via_len
      This specifies the maximum Via field size. If the Via field is greater than 
      this size, an alert is generated. Default is set to 1024. The allowed range 
      for this option is 0 - 65535. "0" means never alert.
  
  max_contact_len
      This specifies the maximum Contact field size. If the Contact field is 
      greater than this size, an alert is generated. Default is set to 256. 
      The allowed range for this option is 0 - 65535. "0" means never alert.
  
  max_content_len
      This specifies the maximum content length of the message body. If the 
      content length is greater than this number, an alert is generated. 
      Default is set to 1024. The allowed range for this option is 0 - 65535. 
      "0" means never alert.

  ignore_call_channel
      This enables the support for ignoring audio/video data channel 
      (through Stream API). By default, this is disabled.

Option examples
     max_sessions 30000
     disabled
     ports { 5060 5061 }
     methods { invite cancel ack bye register options }
     methods { invite cancel ack bye register options information }
     max_uri_len 1024
     max_call_id_len 1024
     max_requestName_len 10
     max_from_len 1024
     max_to_len 1024
     max_via_len 1024
     max_contact_len 1024
     max_content_len 1024
     max_content_len
     ignore_call_channel

Configuration examples
     preprocessor sip
     preprocessor sip: max_sessions 500000
     preprocessor sip: max_contact_len 512, max_sessions 300000, methods { invite \
                       cancel ack bye register options } , ignore_call_channel
     preprocessor sip: ports { 5060 49848 36780 10270 }, max_call_id_len 200, \
                      max_from_len 100, max_to_len 200, max_via_len 1000, \
                      max_requestName_len 50, max_uri_len 100, ignore_call_channel,\
                      max_content_len 1000
     preprocessor sip: disabled
     preprocessor sip: ignore_call_channel

Default configuration
     preprocessor sip

Events
================================================================================
The preprocessor uses GID 140 to register events.
 

SID   Description 
--------------------------------------------------------------------------------
  1   If the memory cap is reached and the preprocessor is configured to alert, 
      this alert will be created.
  2   Request_URI is required. When Request_URI is empty, this alert will be created. 
  3   The Request_URI is larger than the defined length in configuration.
  4   When Call-ID is empty, this alert will be created.
  5   The Call-ID is larger than the defined length in configuration.
  6   The sequence e number value MUST be expressible as a 32-bit unsigned integer
      and MUST be less than 2**31.
  7   The request name in the CSeq is larger than the defined length in configuration.
  8   From field is empty.
  9   From field is larger than the defined length in configuration.
 10   To field is empty.
 11   To field is larger than the defined length in configuration.
 12   Via filed is empty.
 13   Via filed is larger than the defined length in configuration.
 14   Contact is empty, but it is required non-empty for the message. 
 15   The Contact is larger than the defined length in configuration.
 16   The content length is larger than the defined length in configuration or is negative.
 17   There are multiple requests in a single packet. Old SIP protocol supports
      multiple sip messages within one packet.
 18   There are inconsistencies between Content-Length in SIP header and 
      actual body data.
 19   Request name is invalid in response.
 20   Authenticated invite message received, but no challenge from server received. 
      This is the case of InviteReplay billing attack.
 21   Authenticated invite message received, but session information has been changed. 
      This is different from re-INVITE, where the dialog has been established.
      and authenticated. This is can prevent FakeBusy billing attack.
 22   Response status code is not a 3 digit number.
 23   Content type header field is required if the message body is not empty.
 24   SIP version other than 2.0, 1.0, and 1.1 is invalid
 25   Mismatch in Method of request and the CSEQ header
 26   The method is unknown
 27   The number of dialogs in the stream session exceeds the maximal value.
  
Rule Options
================================================================================
New rule options are supported by enabling the sip preprocessor:

sip_method
sip_stat_code
sip_header
sip_body

Overload modifiers to existing pcre rule options:

H: Match SIP request or SIP response header, Similar to sip_header.
P: Match SIP request or SIP response body, Similar to sip_body.

  sip_method
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  The sip_method keyword is used to check for specific SIP request methods.  
  The list of methods is: invite, cancel, ack, bye, register, options, refer, 
  subscribe, update, join, info, message, notify, prack.  More than one method 
  can be specified, via a comma separated list, and are OR'ed together.  
  It will be applied in fast pattern match if available. If the method used 
  in this rule is not listed in the preprocessor configuration, it will be added
  to the preprocessor configuration for the associated policy.
  
  Syntax:
    sip_method:<method-list>;
    method-list = method|method, method-list
    method      = ["!"] "invite" | "cancel" | "ack" | "bye" | "register" | "options"
                  | "refer" | "subscribe" | "update" | "join" | "info" |  "notify" |  
                  | "message"| "prack" 
    Note: if "!" is used, only one method is allowed in sip_method. 
   
  Examples:
   sip_method:invite, cancel
   sip_method:!invite
   
   If a user wants to use "and", they can use something like this:
   sip_method:!invite; sip_method:!bye
   
  sip_stat_code
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  The sip_stat_code is used to check the SIP response status code.  This option
  matches if any one of the state codes specified matches the status codes of
  the SIP response.

  Syntax:
    sip_stat_code:<code _list> ;
    code_list = state_code|state_code, code_list
    code      = "100-999"|"1-9"   
    Note: 1,2,3,4,5,6... mean to check for "1xx", "2xx", "3xx", "4xx", "5xx", 
          ,"6xx"... responses.  
  Example:
    This rule searches for the response with state code "200".
    sip_stat_code:200
    This rule searches for all the 2xx responses.
    sip_stat_code: 2
    This rule searches for either 200, or 180 responses.
    sip_stat_code: 200, 180
  
  sip_header
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   
  The sip_header keyword restricts the search to the extracted Header fields of 
  a SIP message request or a response. 
  
  Syntax:
    sip_header;
  
  Example:
    This rule constrains the search for the pattern "CSeq" to the extracted Header
    fields of a SIP message.
    alert udp any any -> any 5060 (sip_header; content: "CSeq"; )
    
  sip_body
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   
  The sip_body keyword places the cursor at the beginning of the Body fields 
  of a SIP message. This works similar to file_data and dce_stub_data.The message
  body includes channel information using SDP protocol (Session Description Protocol).
  
  Syntax:
    sip_body;
  Example:
    This rule searches for the pattern "c=IN 0.0.0.0" in the Body fields 
    of a SIP message. 
    alert udp any any -> any 5060 (sip_body; content: "C=IN 0.0.0.0"; within 100;)
 
  pcre
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~   
  SIP overloads two options for pcre:
  
  H: Match SIP request or SIP response header, Similar to sip_header.
  P: Match SIP request or SIP response body, Similar to sip_body.
  
  Example:
    This rule searches for the pattern "INVITE" in the Header fields of a SIP message.
    alert udp any any -> any 5060 (pcre:"/INVITE/H"; sid:1000000;)
    This rule searches for the pattern "m=" in the Body fields of a SIP message.
    alert udp any any -> any 5060 (pcre:"/m=/P"; sid:2000000;)
    