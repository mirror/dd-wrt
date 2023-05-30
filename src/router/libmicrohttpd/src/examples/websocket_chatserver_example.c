/*
     This file is part of libmicrohttpd
     Copyright (C) 2021 David Gausmann (and other contributing authors)

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 2.1 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
/**
 * @file websocket_chatserver_example.c
 * @brief example for how to use websockets
 * @author David Gausmann
 *
 * Access the HTTP server with your webbrowser.
 * The webbrowser must support JavaScript and WebSockets.
 * The websocket access will be initiated via the JavaScript on the website.
 * You will get an example chat room, which uses websockets.
 * For testing with multiple users, just start several instances of your webbrowser.
 *
 */

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "platform.h"
#include <microhttpd.h>
#include <microhttpd_ws.h>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
/*
  Workaround for Windows systems, because the NuGet version of pthreads is buggy.
  This is a simple replacement. It doesn't offer all functions of pthread, but
  has everything, what is required for this example.
  See: https://github.com/coapp-packages/pthreads/issues/2
*/
#include "pthread_windows.h"

/*
  On Windows we will use stricmp instead of strcasecmp (strcasecmp is undefined there).
*/
#define strcasecmp stricmp

#else
/*
  On Unix systems we can use pthread.
*/
#include <pthread.h>
#endif


/*
 * Specify with this constant whether or not to use HTTPS.
 * 0 means HTTP, 1 means HTTPS.
 * Please note that you must enter a valid private key/certificate pair
 * in the main procedure to running this example with HTTPS.
 */
#define USE_HTTPS 0

/**
 * This is the main website.
 * The HTML, CSS and JavaScript code is all in there.
 */
#define PAGE \
  "<!DOCTYPE html>" \
  "<html>" \
  "<head>" \
  "<meta charset='UTF-8'>" \
  "<title>libmicrohttpd websocket chatserver demo</title>" \
  "<style>" \
  "  html" \
  "  {\n" \
  "    font: 11pt sans-serif;\n" \
  "  }\n" \
  "  html, body" \
  "  {\n" \
  "    margin: 0;\n" \
  "    width:  100vw;\n" \
  "    height: 100vh;\n" \
  "  }\n" \
  "  div#Chat\n" \
  "  {\n" \
  "    display:        flex;\n" \
  "    flex-direction: row;\n" \
  "  }\n" \
  "  div#Chat > div.MessagesAndInput\n" \
  "  {\n" \
  "    flex:             1 1 auto;" \
  "    display:          flex;\n" \
  "    flex-direction:   column;\n" \
  "    width:            calc(100vw - 20em);\n" \
  "  }\n" \
  "  div#Chat > div.MessagesAndInput > div#Messages\n" \
  "  {\n" \
  "    flex:             1 1 auto;" \
  "    display:          flex;\n" \
  "    flex-direction:   column;\n" \
  "    justify-content:  flex-start;\n" \
  "    box-sizing:       border-box;\n" \
  "    overflow-y:       scroll;\n" \
  "    border:           2pt solid #888;\n" \
  "    background-color: #eee;\n" \
  "    height:           calc(100vh - 2em);\n" \
  "  }\n" \
  "  div#Chat > div.MessagesAndInput > div#Messages > div.Message > span\n" \
  "  {\n" \
  "    white-space: pre\n" \
  "  }\n" \
  "  div#Chat > div.MessagesAndInput > div#Messages > div.Message.error > span\n" \
  "  {\n" \
  "    color: red;\n" \
  "  }\n" \
  "  div#Chat > div.MessagesAndInput > div#Messages > div.Message.system > span\n" \
  "  {\n" \
  "    color: green;\n" \
  "  }\n" \
  "  div#Chat > div.MessagesAndInput > div#Messages > div.Message.moderator > span\n" \
  "  {\n" \
  "    color: #808000;\n" \
  "  }\n" \
  "  div#Chat > div.MessagesAndInput > div#Messages > div.Message.private > span\n" \
  "  {\n" \
  "    color: blue;\n" \
  "  }\n" \
  "  div#Chat > div.MessagesAndInput > div.Input\n" \
  "  {\n" \
  "    flex:             0 0 auto;" \
  "    height:           2em;" \
  "    display:          flex;" \
  "    flex-direction:   row;" \
  "    background-color: #eee;\n" \
  "  }\n" \
  "  div#Chat > div.MessagesAndInput > div.Input > input#InputMessage\n" \
  "  {\n" \
  "    flex:             1 1 auto;" \
  "  }\n" \
  "  div#Chat > div.MessagesAndInput > div.Input > button\n" \
  "  {\n" \
  "    flex:             0 0 auto;" \
  "    width:            5em;" \
  "    margin-left:      4pt;" \
  "  }\n" \
  "  div#Chat > div#Users\n" \
  "  {\n" \
  "    flex:             0 0 auto;" \
  "    width:            20em;" \
  "    display:          flex;\n" \
  "    flex-direction:   column;\n" \
  "    justify-content:  flex-start;\n" \
  "    box-sizing:       border-box;\n" \
  "    overflow-y:       scroll;\n" \
  "    border:           2pt solid #888;\n" \
  "    background-color: #eee;\n" \
  "  }\n" \
  "  div#Chat > div#Users > div\n" \
  "  {\n" \
  "    cursor: pointer;\n" \
  "    user-select: none;\n" \
  "    -webkit-user-select: none;\n" \
  "  }\n" \
  "  div#Chat > div#Users > div.selected\n" \
  "  {\n" \
  "    background-color: #7bf;\n" \
  "  }\n" \
  "</style>" \
  "<script>\n" \
  "  'use strict'\n;" \
  "\n" \
  "  let baseUrl;\n" \
  "  let socket;\n" \
  "  let connectedUsers = new Map();\n" \
  "\n" \
  "  window.addEventListener('load', window_onload);\n" \
  "\n" \
  " /**\n" \
  "    This is the main procedure which initializes the chat and connects the first socket\n" \
  "  */\n" \
  "  function window_onload(event)\n" \
  "  {\n" \
  " /* Determine the base url (for http:/" "/ this is ws:/" "/ for https:/" \
                                  "/ this must be wss:/" "/) */\n" \
  "    baseUrl = 'ws' + (window.location.protocol === 'https:' ? 's' : '') + ':/" "/' + window.location.host + '/ChatServerWebSocket';\n" \
  "    chat_generate();\n" \
  "    chat_connect();\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This function generates the chat using DOM\n" \
  "  */\n" \
  "  function chat_generate()\n" \
  "  {\n" \
  "    document.body.innerHTML = '';\n" \
  "    let chat = document.createElement('div');\n" \
  "    document.body.appendChild(chat);\n" \
  "    chat.id = 'Chat';\n" \
  "    let messagesAndInput = document.createElement('div');\n" \
  "    chat.appendChild(messagesAndInput);\n" \
  "    messagesAndInput.classList.add('MessagesAndInput');\n" \
  "    let messages = document.createElement('div');\n" \
  "    messagesAndInput.appendChild(messages);\n" \
  "    messages.id = 'Messages';\n" \
  "    let input = document.createElement('div');\n" \
  "    messagesAndInput.appendChild(input);\n" \
  "    input.classList.add('Input');\n" \
  "    let inputMessage = document.createElement('input');\n" \
  "    input.appendChild(inputMessage);\n" \
  "    inputMessage.type = 'text';\n" \
  "    inputMessage.id = 'InputMessage';\n" \
  "    inputMessage.disabled = true;\n" \
  "    inputMessage.addEventListener('keydown', chat_onKeyDown);\n" \
  "    let inputMessageSend = document.createElement('button');\n" \
  "    input.appendChild(inputMessageSend);\n" \
  "    inputMessageSend.id = 'InputMessageButton';\n" \
  "    inputMessageSend.disabled = true;\n" \
  "    inputMessageSend.innerText = 'send';\n" \
  "    inputMessageSend.addEventListener('click', chat_onSendClicked);\n" \
  "    let inputImage = document.createElement('input');\n" \
  "    input.appendChild(inputImage);\n" \
  "    inputImage.id = 'InputImage';\n" \
  "    inputImage.type = 'file';\n" \
  "    inputImage.accept = 'image /*';\n" \
  "    inputImage.style.display = 'none';\n" \
  "    inputImage.addEventListener('change', chat_onImageSelected);\n" \
  "    let inputImageButton = document.createElement('button');\n" \
  "    input.appendChild(inputImageButton);\n" \
  "    inputImageButton.id = 'InputImageButton';\n" \
  "    inputImageButton.disabled = true;\n" \
  "    inputImageButton.innerText = 'image';\n" \
  "    inputImageButton.addEventListener('click', chat_onImageClicked);\n" \
  "    let users = document.createElement('div');\n" \
  "    chat.appendChild(users);\n" \
  "    users.id = 'Users';\n" \
  "    users.addEventListener('click', chat_onUserClicked);\n" \
  "    let allUsers = document.createElement('div');\n" \
  "    users.appendChild(allUsers);\n" \
  "    allUsers.classList.add('selected');\n" \
  "    allUsers.innerText = '<everyone>';\n" \
  "    allUsers.setAttribute('data-user', '0');\n" \
  "  }\n" \
  "\n" \
  "  /**\n" \
  "    This function creates and connects a WebSocket\n" \
  "  */\n" \
  "  function chat_connect()\n" \
  "  {\n" \
  "    chat_addMessage(`Connecting to libmicrohttpd chat server demo (${baseUrl})...`, { type: 'system' });\n" \
  "    socket = new WebSocket(baseUrl);\n" \
  "    socket.binaryType = 'arraybuffer';\n" \
  "    socket.onopen    = socket_onopen;\n" \
  "    socket.onclose   = socket_onclose;\n" \
  "    socket.onerror   = socket_onerror;\n" \
  "    socket.onmessage = socket_onmessage;\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This function adds new text to the chat list\n" \
  "  */\n" \
  "  function chat_addMessage(text, options)\n" \
  "  {\n" \
  "    let type = options && options.type || 'regular';\n" \
  "    if(!/^(?:regular|system|error|private|moderator)$/.test(type))\n" \
  "      type = 'regular';\n" \
  "    let message = document.createElement('div');\n" \
  "    message.classList.add('Message');\n" \
  "    message.classList.add(type);\n" \
  "    if(typeof(text) === 'string')\n" \
  "    {\n" \
  "      let content = document.createElement('span');\n" \
  "      message.appendChild(content);\n" \
  "      if(options && options.from)\n" \
  "        content.innerText = `${options.from}: ${text}`;\n" \
  "      else\n" \
  "        content.innerText = text;\n" \
  "      if(options && options.reconnect)\n" \
  "      {\n" \
  "        let span = document.createElement('span');\n" \
  "        span.appendChild(document.createTextNode(' ('));\n" \
  "        let reconnect = document.createElement('a');\n" \
  "        reconnect.href = 'javascript:chat_connect()';\n" \
  "        reconnect.innerText = 'reconnect';\n" \
  "        span.appendChild(reconnect);\n" \
  "        span.appendChild(document.createTextNode(')'));\n" \
  "        message.appendChild(span);\n" \
  "      }\n" \
  "    }\n" \
  "    else\n" \
  "    {\n" \
  "      let content = document.createElement('span');\n" \
  "      message.appendChild(content);\n" \
  "      if(options && options.from)\n" \
  "      {\n" \
  "        content.innerText = `${options.from}:\\n`;\n" \
  "      }\n" \
  "      if(options && options.pictureType && text instanceof Uint8Array)\n" \
  "      {\n" \
  "        let img = document.createElement('img');\n" \
  "        content.appendChild(img);\n" \
  "        img.src = URL.createObjectURL(new Blob([ text.buffer ], { type: options.pictureType }));\n" \
  "      }\n" \
  "    }\n" \
  "    document.getElementById('Messages').appendChild(message);\n" \
  "    message.scrollIntoView();\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This is a keydown event handler, which allows that you can just press enter instead of clicking the 'send' button\n" \
  "  */\n" \
  "  function chat_onKeyDown(event)\n" \
  "  {\n" \
  "    if(event.key == 'Enter')\n" \
  "      chat_onSendClicked();\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This is the code to send a message or command, when clicking the 'send' button\n" \
  "  */\n" \
  "  function chat_onSendClicked(event)\n" \
  "  {\n" \
  "    let message = document.getElementById('InputMessage').value;\n" \
  "    if(message.length == 0)\n" \
  "      return;\n" \
  "    if(message.substr(0, 1) == '/')\n" \
  "    {\n" \
  " /* command */ \n"     \
  "      let match;\n" \
  "      if(/^\\/disconnect\\s*$/.test(message))\n" \
  "      {\n" \
  "        socket.close(1000);\n" \
  "      }\n" \
  "      else if((match = /^\\/m\\s+(\\S+)\\s+/.exec(message)))\n" \
  "      {\n" \
  "        message = message.substr(match[0].length);\n" \
  "        let userId = chat_getUserIdByName(match[1]);\n" \
  "        if(userId !== null)\n" \
  "        {\n" \
  "          socket.send(`private|${userId}|${message}`);\n" \
  "        }\n" \
  "        else\n" \
  "        {\n" \
  "          chat_addMessage(`Unknown user \"${match[1]}\" for private message: ${message}`, { type: 'error' });\n" \
  "        }\n" \
  "      }\n" \
  "      else if((match = /^\\/ping\\s+(\\S+)\\s*$/.exec(message)))\n" \
  "      {\n" \
  "        let userId = chat_getUserIdByName(match[1]);\n" \
  "        if(userId !== null)\n" \
  "        {\n" \
  "          socket.send(`ping|${userId}|`);\n" \
  "        }\n" \
  "        else\n" \
  "        {\n" \
  "          chat_addMessage(`Unknown user \"${match[1]}\" for ping`, { type: 'error' });\n" \
  "        }\n" \
  "      }\n" \
  "      else if((match = /^\\/name\\s+(\\S+)\\s*$/.exec(message)))\n" \
  "      {\n" \
  "        socket.send(`name||${match[1]}`);\n" \
  "      }\n" \
  "      else\n" \
  "      {\n" \
  "        chat_addMessage(`Unsupported command or invalid syntax: ${message}`, { type: 'error' });\n" \
  "      }\n" \
  "    }\n" \
  "    else\n" \
  "    {\n" \
  " /* regular chat message to the selected user */ \n"     \
  "      let selectedUser = document.querySelector('div#Users > div.selected');\n" \
  "      let selectedUserId = parseInt(selectedUser.getAttribute('data-user') || '0', 10);\n" \
  "      if(selectedUserId == 0)\n" \
  "        socket.send(`||${message}`);\n" \
  "      else\n" \
  "        socket.send(`private|${selectedUserId}|${message}`);\n" \
  "    }\n" \
  "    document.getElementById('InputMessage').value = '';\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This is the event when the user hits the 'image' button\n" \
  "  */\n" \
  "  function chat_onImageClicked(event)\n" \
  "  {\n" \
  "    document.getElementById('InputImage').click();\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This is the event when the user selected an image.\n" \
  "    The image will be read with the FileReader (allowed in web, because the user selected the file).\n" \
  "  */\n" \
  "  function chat_onImageSelected(event)\n" \
  "  {\n" \
  "    let file = event.target.files[0];\n" \
  "    if(!file || !/^image\\/" "/.test(file.type))\n" \
  "      return;\n" \
  "    let selectedUser = document.querySelector('div#Users > div.selected');\n" \
  "    let selectedUserId = parseInt(selectedUser.getAttribute('data-user') || '0', 10);\n" \
  "    let reader = new FileReader();\n" \
  "    reader.onload = function(event) {\n" \
  "      chat_onImageRead(event, file.type, selectedUserId);\n" \
  "    };\n" \
  "    reader.readAsArrayBuffer(file);\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This is the event when the user selected image has been read.\n" \
  "    This will add our chat protocol prefix and send it via the websocket.\n" \
  "  */\n" \
  "  function chat_onImageRead(event, fileType, selectedUserId)\n" \
  "  {\n" \
  "    let encoder = new TextEncoder();\n" \
  "    let prefix = ((selectedUserId == 0 ? '||' : `private|${selectedUserId}|`) + fileType + '|');\n" \
  "    prefix = encoder.encode(prefix);\n" \
  "    let byteData = new Uint8Array(event.target.result);\n" \
  "    let totalLength = prefix.length + byteData.length;\n" \
  "    let resultByteData = new Uint8Array(totalLength);\n" \
  "    resultByteData.set(prefix, 0);\n" \
  "    resultByteData.set(byteData, prefix.length);\n" \
  "    socket.send(resultByteData);\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This is the event when the user clicked a name in the user list.\n" \
  "    This is useful to send private messages or images without needing to add the /m prefix.\n" \
  "  */\n" \
  "  function chat_onUserClicked(event, selectedUserId)\n" \
  "  {\n" \
  "    let newSelected = event.target.closest('div#Users > div');\n" \
  "    if(newSelected === null)\n" \
  "      return;\n" \
  "    for(let div of this.querySelectorAll(':scope > div.selected'))\n" \
  "      div.classList.remove('selected');\n" \
  "    newSelected.classList.add('selected');\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This functions returns the current id of a user identified by its name.\n" \
  "  */\n" \
  "  function chat_getUserIdByName(name)\n" \
  "  {\n" \
  "    let nameUpper = name.toUpperCase();\n" \
  "    for(let pair of connectedUsers)\n" \
  "    {\n" \
  "      if(pair[1].toUpperCase() == nameUpper)\n" \
  "        return pair[0];\n" \
  "    }\n" \
  "    return null;\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This functions clears the entire user list (needed for reconnecting).\n" \
  "  */\n" \
  "  function chat_clearUserList()\n" \
  "  {\n" \
  "    let users = document.getElementById('Users');\n" \
  "    for(let div of users.querySelectorAll(':scope > div'))\n" \
  "    {\n" \
  "      if(div.getAttribute('data-user') === '0')\n" \
  "      {\n" \
  "        div.classList.add('selected');\n" \
  "      }\n" \
  "      else\n" \
  "      {\n" \
  "        div.parentNode.removeChild(div);\n" \
  "      }\n" \
  "    }\n" \
  "    return null;\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This is the event when the socket has established a connection.\n" \
  "    This will initialize an empty chat and enable the controls.\n" \
  "  */\n" \
  "  function socket_onopen(event)\n" \
  "  {\n" \
  "    connectedUsers.clear();\n" \
  "    chat_clearUserList();\n" \
  "    chat_addMessage('Connected!', { type: 'system' });\n" \
  "    document.getElementById('InputMessage').disabled       = false;\n" \
  "    document.getElementById('InputMessageButton').disabled = false;\n" \
  "    document.getElementById('InputImageButton').disabled   = false;\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This is the event when the socket has been closed.\n" \
  "    This will lock the controls.\n" \
  "  */\n" \
  "  function socket_onclose(event)\n" \
  "  {\n" \
  "    chat_addMessage('Connection closed!', { type: 'system', reconnect: true });\n" \
  "    document.getElementById('InputMessage').disabled       = true;\n" \
  "    document.getElementById('InputMessageButton').disabled = true;\n" \
  "    document.getElementById('InputImageButton').disabled   = true;\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This is the event when the socket reported an error.\n" \
  "    This will just make an output.\n" \
  "    In the web browser console (F12 on many browsers) will show you more detailed error information.\n" \
  "  */\n" \
  "  function socket_onerror(event)\n" \
  "  {\n" \
  "    console.error('WebSocket error reported: ', event);\n" \
  "    chat_addMessage('The socket reported an error!', { type: 'error' });\n" \
  "  }\n" \
  "\n" \
  " /**\n" \
  "    This is the event when the socket has received a message.\n" \
  "    This will parse the message and execute the corresponding command (or add the message).\n" \
  "  */\n" \
  "  function socket_onmessage(event)\n" \
  "  {\n" \
  "    if(typeof(event.data) === 'string')\n" \
  "    {\n" \
  " /* text message or command */ \n"     \
  "      let message = event.data.split('|', 3);\n" \
  "      switch(message[0])\n" \
  "      {\n" \
  "      case 'userinit':\n" \
  "        connectedUsers.set(message[1], message[2]);\n" \
  "        {\n" \
  "          let users = document.getElementById('Users');\n" \
  "          let div = document.createElement('div');\n" \
  "          users.appendChild(div);\n" \
  "          div.innerText = message[2];\n" \
  "          div.setAttribute('data-user', message[1]);\n" \
  "        }\n" \
  "        break;\n" \
  "      case 'useradd':\n" \
  "        connectedUsers.set(message[1], message[2]);\n" \
  "        chat_addMessage(`The user '${message[2]}' has joined our lovely chatroom.`, { type: 'moderator' });\n" \
  "        {\n" \
  "          let users = document.getElementById('Users');\n" \
  "          let div = document.createElement('div');\n" \
  "          users.appendChild(div);\n" \
  "          div.innerText = message[2];\n" \
  "          div.setAttribute('data-user', message[1]);\n" \
  "        }\n" \
  "        break;\n" \
  "      case 'userdel':\n" \
  "        chat_addMessage(`The user '${connectedUsers.get(message[1])}' has left our chatroom. We will miss you.`, { type: 'moderator' });\n" \
  "        connectedUsers.delete(message[1]);\n" \
  "        {\n" \
  "          let users = document.getElementById('Users');\n" \
  "          let div = users.querySelector(`div[data-user='${message[1]}']`);\n" \
  "          if(div !== null)\n" \
  "          {\n" \
  "            users.removeChild(div);\n" \
  "            if(div.classList.contains('selected'))\n" \
  "              users.querySelector('div[data-user=\\'0\\']').classList.add('selected');\n" \
  "          }\n" \
  "        }\n" \
  "        break;\n" \
  "      case 'username':\n" \
  "        chat_addMessage(`The user '${connectedUsers.get(message[1])}' has changed his name to '${message[2]}'.`, { type: 'moderator' });\n" \
  "        connectedUsers.set(message[1], message[2]);\n" \
  "        {\n" \
  "          let users = document.getElementById('Users');\n" \
  "          let div = users.querySelector(`div[data-user='${message[1]}']`);\n" \
  "          if(div !== null)\n" \
  "          {\n" \
  "            div.innerText = message[2];\n" \
  "          }\n" \
  "        }\n" \
  "        break;\n" \
  "      case 'ping':\n" \
  "        chat_addMessage(`The user '${connectedUsers.get(message[1])}' has a ping of ${message[2]} ms.`, { type: 'moderator' });\n" \
  "        break;\n" \
  "      default:\n" \
  "        chat_addMessage(message[2], { type: message[0], from: connectedUsers.get(message[1]) });\n" \
  "        break;\n" \
  "      }\n" \
  "    }\n" \
  "    else\n" \
  "    {\n" \
  " /* We received a binary frame, which means a picture here */ \n"     \
  "      let byteData = new Uint8Array(event.data);\n" \
  "      let decoder = new TextDecoder();\n" \
  "      let message  = [ ];\n" \
  " /* message type */ \n"     \
  "      let j = 0;\n" \
  "      let i = byteData.indexOf(0x7C, j); /* | = 0x7C;*/ \n"\
  "      if(i < 0)\n" \
  "        return;\n" \
  "      message.push(decoder.decode(byteData.slice(0, i)));\n" \
  " /* picture from */ \n"     \
  "      j = i + 1;\n" \
  "      i = byteData.indexOf(0x7C, j);\n" \
  "      if(i < 0)\n" \
  "        return;\n" \
  "      message.push(decoder.decode(byteData.slice(j, i)));\n" \
  " /* picture encoding */ \n"     \
  "      j = i + 1;\n" \
  "      i = byteData.indexOf(0x7C, j);\n" \
  "      if(i < 0)\n" \
  "        return;\n" \
  "      message.push(decoder.decode(byteData.slice(j, i)));\n" \
  " /* picture */ \n"     \
  "      byteData = byteData.slice(i + 1);\n" \
  "      chat_addMessage(byteData, { type: message[0], from: connectedUsers.get(message[1]), pictureType: message[2] });\n" \
  "    }\n" \
  "  }\n" \
  "</script>" \
  "</head>" \
  "<body><noscript>Please enable JavaScript to test the libmicrohttpd Websocket chatserver demo!</noscript></body>" \
  "</html>"

#define PAGE_NOT_FOUND \
  "404 Not Found"

#define PAGE_INVALID_WEBSOCKET_REQUEST \
  "Invalid WebSocket request!"

/**
 * This struct is used to keep the data of a connected chat user.
 * It is passed to the socket-receive thread (connecteduser_receive_messages) as well as to
 * the socket-send thread (connecteduser_send_messages).
 * It can also be accessed via the global array users (mutex protected).
 */
struct ConnectedUser
{
  /* the TCP/IP socket for reading/writing */
  MHD_socket fd;
  /* the UpgradeResponseHandle of libmicrohttpd (needed for closing the socket) */
  struct MHD_UpgradeResponseHandle *urh;
  /* the websocket encode/decode stream */
  struct MHD_WebSocketStream *ws;
  /* the possibly read data at the start (only used once) */
  char *extra_in;
  size_t extra_in_size;
  /* the unique user id (counting from 1, ids will never be re-used) */
  size_t user_id;
  /* the current user name */
  char *user_name;
  size_t user_name_len;
  /* the zero-based index of the next message;
     may be decremented when old messages are deleted */
  size_t next_message_index;
  /* specifies whether the websocket shall be closed (1) or not (0) */
  int disconnect;
  /* condition variable to wake up the sender of this connection */
  pthread_cond_t wake_up_sender;
  /* mutex to ensure that no send actions are mixed
     (sending can be done by send and recv thread;
      may not be simultaneously locked with chat_mutex by the same thread) */
  pthread_mutex_t send_mutex;
  /* specifies whether a ping shall be executed (1), is being executed (2) or
     no ping is pending (0) */
  int ping_status;
  /* the start time of the ping, if a ping is running */
  struct timespec ping_start;
  /* the message used for the ping (must match the pong response)*/
  char ping_message[128];
  /* the length of the ping message (may not exceed 125) */
  size_t ping_message_len;
  /* the numeric ping message suffix to detect ping messages, which are too old */
  int ping_counter;
};

/**
 * A single message, which has been send via the chat.
 * This can be text, an image or a command.
 */
struct Message
{
  /* The user id of the sender. This is 0 if it is a system message- */
  size_t from_user_id;
  /* The user id of the recipient. This is 0 if every connected user shall receive it */
  size_t to_user_id;
  /* The data of the message. */
  char *data;
  size_t data_len;
  /* Specifies whether the data is UTF-8 encoded text (0) or binary data (1) */
  int is_binary;
};

/* the unique user counter for new users (only accessed by main thread) */
size_t unique_user_id = 0;

/* the chat data (users and messages; may be accessed by all threads, but is protected by mutex) */
pthread_mutex_t chat_mutex;
struct ConnectedUser **users = NULL;
size_t user_count = 0;
struct Message **messages = NULL;
size_t message_count = 0;
/* specifies whether all websockets must close (1) or not (0) */
volatile int disconnect_all = 0;
/* a counter for cleaning old messages (each 10 messages we will try to clean the list */
int clean_count = 0;
#define CLEANUP_LIMIT 10

/**
 * Change socket to blocking.
 *
 * @param fd the socket to manipulate
 */
static void
make_blocking (MHD_socket fd)
{
#if defined(MHD_POSIX_SOCKETS)
  int flags;

  flags = fcntl (fd, F_GETFL);
  if (-1 == flags)
    abort ();
  if ((flags & ~O_NONBLOCK) != flags)
    if (-1 == fcntl (fd, F_SETFL, flags & ~O_NONBLOCK))
      abort ();
#elif defined(MHD_WINSOCK_SOCKETS)
  unsigned long flags = 0;

  if (0 != ioctlsocket (fd, (int) FIONBIO, &flags))
    abort ();
#endif /* MHD_WINSOCK_SOCKETS */
}


/**
 * Sends all data of the given buffer via the TCP/IP socket
 *
 * @param fd  The TCP/IP socket which is used for sending
 * @param buf The buffer with the data to send
 * @param len The length in bytes of the data in the buffer
 */
static void
send_all (struct ConnectedUser *cu,
          const char *buf,
          size_t len)
{
  ssize_t ret;
  size_t off;

  if (0 == pthread_mutex_lock (&cu->send_mutex))
  {
    for (off = 0; off < len; off += ret)
    {
      ret = send (cu->fd,
                  &buf[off],
                  (int) (len - off),
                  0);
      if (0 > ret)
      {
        if (EAGAIN == errno)
        {
          ret = 0;
          continue;
        }
        break;
      }
      if (0 == ret)
        break;
    }
    pthread_mutex_unlock (&cu->send_mutex);
  }
}


/**
 * Adds a new chat message to the list of messages.
 *
 * @param from_user_id the user id of the sender (0 means system)
 * @param to_user_id   the user id of the recipiend (0 means everyone)
 * @param data         the data to send (UTF-8 text or binary; will be copied)
 * @param data_len     the length of the data to send
 * @param is_binary    specifies whether the data is UTF-8 text (0) or binary (1)
 * @param needs_lock   specifies whether the caller has already locked the global chat mutex (0) or
 *                     if this procedure needs to lock it (1)
 *
 * @return             0 on success, other values on error
 */
static int
chat_addmessage (size_t from_user_id,
                 size_t to_user_id,
                 char *data,
                 size_t data_len,
                 int is_binary,
                 int needs_lock)
{
  /* allocate the buffer and fill it with data */
  struct Message *message = (struct Message *) malloc (sizeof (struct Message));
  if (NULL == message)
    return 1;

  memset (message, 0, sizeof (struct Message));
  message->from_user_id = from_user_id;
  message->to_user_id   = to_user_id;
  message->is_binary    = is_binary;
  message->data_len     = data_len;
  message->data = malloc (data_len + 1);
  if (NULL == message->data)
  {
    free (message);
    return 1;
  }
  memcpy (message->data, data, data_len);
  message->data[data_len] = 0;

  /* lock the global mutex if needed */
  if (0 != needs_lock)
  {
    if (0 != pthread_mutex_lock (&chat_mutex))
      return 1;
  }

  /* add the new message to the global message list */
  size_t message_count_ = message_count + 1;
  struct Message **messages_ = (struct Message **) realloc (messages,
                                                            message_count_
                                                            * sizeof (struct
                                                                      Message *));
  if (NULL == messages_)
  {
    free (message);
    if (0 != needs_lock)
      pthread_mutex_unlock (&chat_mutex);
    return 1;
  }
  messages_[message_count] = message;
  messages = messages_;
  message_count = message_count_;

  /* inform the sender threads about the new message */
  for (size_t i = 0; i < user_count; ++i)
    pthread_cond_signal (&users[i]->wake_up_sender);

  /* unlock the global mutex if needed */
  if (0 != needs_lock)
  {
    if (0 != needs_lock)
      pthread_mutex_unlock (&chat_mutex);
  }
  return 0;
}


/**
 * Cleans up old messages
 *
 * @param needs_lock   specifies whether the caller has already locked the global chat mutex (0) or
 *                     if this procedure needs to lock it (1)
 * @return             0 on success, other values on error
 */
static int
chat_clearmessages (int needs_lock)
{
  /* lock the global mutex if needed */
  if (0 != needs_lock)
  {
    if (0 != pthread_mutex_lock (&chat_mutex))
      return 1;
  }

  /* update the clean counter and check whether we need cleaning */
  ++clean_count;
  if (CLEANUP_LIMIT > clean_count)
  {
    /* no cleanup required */
    if (0 != needs_lock)
    {
      pthread_mutex_unlock (&chat_mutex);
    }
    return 0;
  }
  clean_count = 0;

  /* check whether we got any messages (without them no cleaning is required */
  if (0 < message_count)
  {
    /* then check whether we got any connected users */
    if (0 < user_count)
    {
      /* determine the minimum index for the next message of all connected users */
      size_t min_message = users[0]->next_message_index;
      for (size_t i = 1; i < user_count; ++i)
      {
        if (min_message > users[i]->next_message_index)
          min_message = users[i]->next_message_index;
      }
      if (0 < min_message)
      {
        /* remove all messages with index below min_message and update
           the message indices of the users */
        for (size_t i = 0; i < min_message; ++i)
        {
          free (messages[i]->data);
          free (messages[i]);
        }
        for (size_t i = min_message; i < message_count; ++i)
          messages[i - min_message] = messages[i];
        message_count -= min_message;
        for (size_t i = 0; i < user_count; ++i)
          users[i]->next_message_index -= min_message;
      }
    }
    else
    {
      /* without connected users, simply remove all messages */
      for (size_t i = 0; i < message_count; ++i)
      {
        free (messages[i]->data);
        free (messages[i]);
      }
      free (messages);
      messages = NULL;
      message_count = 0;
    }
  }

  /* unlock the global mutex if needed */
  if (0 != needs_lock)
  {
    pthread_mutex_unlock (&chat_mutex);
  }
  return 0;
}


/**
 * Adds a new chat user to the global user list.
 * This will be called at the start of connecteduser_receive_messages.
 *
 * @param cu The connected user
 * @return   0 on success, other values on error
 */
static int
chat_adduser (struct ConnectedUser *cu)
{
  /* initialize the notification message of the new user */
  char user_index[32];
  snprintf (user_index, 32, "%d", (int) cu->user_id);
  size_t user_index_len = strlen (user_index);
  size_t data_len = user_index_len + cu->user_name_len + 9;
  char *data = (char *) malloc (data_len + 1);
  if (NULL == data)
    return 1;
  strcpy (data, "useradd|");
  strcat (data, user_index);
  strcat (data, "|");
  strcat (data, cu->user_name);

  /* lock the mutex */
  if (0 != pthread_mutex_lock (&chat_mutex))
  {
    free (data);
    return 1;
  }
  /* inform the other chat users about the new user */
  if (0 != chat_addmessage (0,
                            0,
                            data,
                            data_len,
                            0,
                            0))
  {
    free (data);
    pthread_mutex_unlock (&chat_mutex);
    return 1;
  }
  free (data);

  /* add the new user to the list */
  size_t user_count_ = user_count + 1;
  struct ConnectedUser **users_ = (struct ConnectedUser **) realloc (users,
                                                                     user_count_
                                                                     * sizeof (
                                                                       struct
                                                                       ConnectedUser
                                                                       *));
  if (NULL == users_)
  {
    /* realloc failed */
    pthread_mutex_unlock (&chat_mutex);
    return 1;
  }
  users_[user_count] = cu;
  users      = users_;
  user_count = user_count_;

  /* Initialize the next message index to the current message count. */
  /* This will skip all old messages for this new connected user. */
  cu->next_message_index = message_count;

  /* unlock the mutex */
  pthread_mutex_unlock (&chat_mutex);
  return 0;
}


/**
 * Removes a chat user from the global user list.
 *
 * @param cu The connected user
 * @return   0 on success, other values on error
 */
static int
chat_removeuser (struct ConnectedUser *cu)
{
  char user_index[32];

  /* initialize the chat message for the removed user */
  snprintf (user_index, 32, "%d", (int) cu->user_id);
  size_t user_index_len = strlen (user_index);
  size_t data_len = user_index_len + 9;
  char *data = (char *) malloc (data_len + 1);
  if (NULL == data)
    return 1;
  strcpy (data, "userdel|");
  strcat (data, user_index);
  strcat (data, "|");

  /* lock the mutex */
  if (0 != pthread_mutex_lock (&chat_mutex))
  {
    free (data);
    return 1;
  }
  /* inform the other chat users that the user is gone */
  int got_error = 0;
  if (0 != chat_addmessage (0, 0, data, data_len, 0, 0))
  {
    free (data);
    got_error = 1;
  }

  /* remove the user from the list */
  int found = 0;
  for (size_t i = 0; i < user_count; ++i)
  {
    if (cu == users[i])
    {
      found = 1;
      for (size_t j = i + 1; j < user_count; ++j)
      {
        users[j - 1] = users[j];
      }
      --user_count;
      break;
    }
  }
  if (0 == found)
    got_error = 1;

  /* unlock the mutex */
  pthread_mutex_unlock (&chat_mutex);

  return got_error;
}


/**
 * Renames a chat user
 *
 * @param cu           The connected user
 * @param new_name     The new user name. On success this pointer will be taken.
 * @param new_name_len The length of the new name
 * @return             0 on success, other values on error. 2 means name already in use.
 */
static int
chat_renameuser (struct ConnectedUser *cu,
                 char *new_name,
                 size_t new_name_len)
{
  /* lock the mutex */
  if (0 != pthread_mutex_lock (&chat_mutex))
  {
    return 1;
  }

  /* check whether the name is already in use */
  for (size_t i = 0; i < user_count; ++i)
  {
    if (cu != users[i])
    {
      if ((users[i]->user_name_len == new_name_len) &&
          (0 == strcasecmp (users[i]->user_name, new_name)))
      {
        pthread_mutex_unlock (&chat_mutex);
        return 2;
      }
    }
  }

  /* generate the notification message */
  char user_index[32];
  snprintf (user_index, 32, "%d", (int) cu->user_id);
  size_t user_index_len = strlen (user_index);
  size_t data_len = user_index_len + new_name_len + 10;
  char *data = (char *) malloc (data_len + 1);
  if (NULL == data)
    return 1;
  strcpy (data, "username|");
  strcat (data, user_index);
  strcat (data, "|");
  strcat (data, new_name);

  /* inform the other chat users about the new name */
  if (0 != chat_addmessage (0, 0, data, data_len, 0, 0))
  {
    free (data);
    pthread_mutex_unlock (&chat_mutex);
    return 1;
  }
  free (data);

  /* accept the new user name */
  free (cu->user_name);
  cu->user_name = new_name;
  cu->user_name_len = new_name_len;

  /* unlock the mutex */
  pthread_mutex_unlock (&chat_mutex);

  return 0;
}


/**
* Parses received data from the TCP/IP socket with the websocket stream
*
* @param cu           The connected user
* @param new_name     The new user name
* @param new_name_len The length of the new name
* @return             0 on success, other values on error
*/
static int
connecteduser_parse_received_websocket_stream (struct ConnectedUser *cu,
                                               char *buf,
                                               size_t buf_len)
{
  size_t buf_offset = 0;
  while (buf_offset < buf_len)
  {
    size_t new_offset = 0;
    char *frame_data = NULL;
    size_t frame_len  = 0;
    int status = MHD_websocket_decode (cu->ws,
                                       buf + buf_offset,
                                       buf_len - buf_offset,
                                       &new_offset,
                                       &frame_data,
                                       &frame_len);
    if (0 > status)
    {
      /* an error occurred and the connection must be closed */
      if (NULL != frame_data)
      {
        /* depending on the WebSocket flag */
        /* MHD_WEBSOCKET_FLAG_GENERATE_CLOSE_FRAMES_ON_ERROR */
        /* close frames might be generated on errors */
        send_all (cu,
                  frame_data,
                  frame_len);
        MHD_websocket_free (cu->ws, frame_data);
      }
      return 1;
    }
    else
    {
      buf_offset += new_offset;

      if (0 < status)
      {
        /* the frame is complete */
        switch (status)
        {
        case MHD_WEBSOCKET_STATUS_TEXT_FRAME:
        case MHD_WEBSOCKET_STATUS_BINARY_FRAME:
          /**
           * a text or binary frame has been received.
           * in this chat server example we use a simple protocol where
           * the JavaScript added a prefix like "<command>|<to_user_id>|data".
           * Some examples:
           * "||test" means a regular chat message to everyone with the message "test".
           * "private|1|secret" means a private chat message to user with id 1 with the message "secret".
           * "name||MyNewName" means that the user requests a rename to "MyNewName"
           * "ping|1|" means that the user with id 1 shall get a ping
           *
           * Binary data is handled here like text data.
           * The difference in the data is only checked by the JavaScript.
           */
          {
            size_t command      = 1000;
            size_t from_user_id = cu->user_id;
            size_t to_user_id   = 0;
            size_t i;

            /* parse the command */
            for (i = 0; i < frame_len; ++i)
            {
              if ('|' == frame_data[i])
              {
                frame_data[i] = 0;
                ++i;
                break;
              }
            }
            if (0 < i)
            {
              if (i == 1)
              {
                /* no command means regular message */
                command = 0;
              }
              else if (0 == strcasecmp (frame_data, "private"))
              {
                /* private means private message */
                command = 1;
              }
              else if (0 == strcasecmp (frame_data, "name"))
              {
                /* name means chat user rename */
                command = 2;
              }
              else if (0 == strcasecmp (frame_data, "ping"))
              {
                /* ping means a ping request */
                command = 3;
              }
              else
              {
                /* no other commands supported, so this means invalid */
                command = 1000;
              }
            }

            /* parse the to_user_id, if given */
            size_t j = i;
            for (; j < frame_len; ++j)
            {
              if ('|' == frame_data[j])
              {
                frame_data[j] = 0;
                ++j;
                break;
              }
            }
            if (i + 1 < j)
            {
              to_user_id = (size_t) atoi (frame_data + i);
            }

            /* decide via the command what action to do */
            if (frame_len >= j)
            {
              int is_binary = (MHD_WEBSOCKET_STATUS_BINARY_FRAME == status ? 1 :
                               0);
              switch (command)
              {
              case 0:
                /* regular chat message */
                {
                  /**
                  * Generate the message for the message list.
                  * Regular chat messages get the command "regular".
                  * After that we add the from_user_id, followed by the content.
                  * The content must always be copied with memcpy instead of strcat,
                  * because the data (binary as well as UTF-8 encoded) is allowed
                  * to contain the NUL character.
                  * However we will add a terminating NUL character,
                  * which is not included in the data length
                  * (and thus will not be send to the recipients).
                  * This is useful for debugging with an IDE.
                  */
                  char user_index[32];
                  snprintf (user_index, 32, "%d", (int) cu->user_id);
                  size_t user_index_len = strlen (user_index);
                  size_t data_len = user_index_len + frame_len - j + 9;
                  char *data = (char *) malloc (data_len + 1);
                  if (NULL != data)
                  {
                    strcpy (data, "regular|");
                    strcat (data, user_index);
                    strcat (data, "|");
                    size_t offset = strlen (data);
                    memcpy (data + offset,
                            frame_data + j,
                            frame_len - j);
                    data[data_len] = 0;

                    /* add the chat message to the global list */
                    chat_addmessage (from_user_id,
                                     0,
                                     data,
                                     data_len,
                                     is_binary,
                                     1);
                    free (data);
                  }
                }
                break;

              case 1:
                /* private chat message */
                if (0 != to_user_id)
                {
                  /**
                   * Generate the message for the message list.
                   * This is similar to the code for regular messages above.
                   * The difference is the prefix "private"
                  */
                  char user_index[32];
                  snprintf (user_index, 32, "%d", (int) cu->user_id);
                  size_t user_index_len = strlen (user_index);
                  size_t data_len = user_index_len + frame_len - j + 9;
                  char *data = (char *) malloc (data_len + 1);
                  if (NULL != data)
                  {

                    strcpy (data, "private|");
                    strcat (data, user_index);
                    strcat (data, "|");
                    size_t offset = strlen (data);
                    memcpy (data + offset,
                            frame_data + j,
                            frame_len - j);
                    data[data_len] = 0;

                    /* add the chat message to the global list */
                    chat_addmessage (from_user_id,
                                     to_user_id,
                                     data,
                                     data_len,
                                     is_binary,
                                     1);
                    free (data);
                  }
                }
                break;

              case 2:
                /* rename */
                {
                  /* check whether the new name is valid and allocate a new buffer for it */
                  size_t new_name_len = frame_len - j;
                  if (0 == new_name_len)
                  {
                    chat_addmessage (0,
                                     from_user_id,
                                     "error||Your new name is invalid. You haven't been renamed.",
                                     58,
                                     0,
                                     1);
                    break;
                  }
                  char *new_name = (char *) malloc (new_name_len + 1);
                  if (NULL == new_name)
                  {
                    chat_addmessage (0,
                                     from_user_id,
                                     "error||Error while renaming. You haven't been renamed.",
                                     54,
                                     0,
                                     1);
                    break;
                  }
                  new_name[new_name_len] = 0;
                  for (size_t k = 0; k < new_name_len; ++k)
                  {
                    char c = frame_data[j + k];
                    if ((32 >= c) || (c >= 127))
                    {
                      free (new_name);
                      new_name = NULL;
                      chat_addmessage (0,
                                       from_user_id,
                                       "error||Your new name contains invalid characters. You haven't been renamed.",
                                       75,
                                       0,
                                       1);
                      break;
                    }
                    new_name[k] = c;
                  }
                  if (NULL == new_name)
                    break;

                  /* rename the user */
                  int rename_result = chat_renameuser (cu,
                                                       new_name,
                                                       new_name_len);
                  if (0 != rename_result)
                  {
                    /* the buffer will only be freed if no rename was possible */
                    free (new_name);
                    if (2 == rename_result)
                    {
                      chat_addmessage (0,
                                       from_user_id,
                                       "error||Your new name is already in use by another user. You haven't been renamed.",
                                       81,
                                       0,
                                       1);
                    }
                    else
                    {
                      chat_addmessage (0,
                                       from_user_id,
                                       "error||Error while renaming. You haven't been renamed.",
                                       54,
                                       0,
                                       1);
                    }
                  }
                }
                break;

              case 3:
                /* ping */
                {
                  if (0 == pthread_mutex_lock (&chat_mutex))
                  {
                    /* check whether the to_user exists */
                    struct ConnectedUser *ping_user = NULL;
                    for (size_t k = 0; k < user_count; ++k)
                    {
                      if (users[k]->user_id == to_user_id)
                      {
                        ping_user = users[k];
                        break;
                      }
                    }
                    if (NULL == ping_user)
                    {
                      chat_addmessage (0,
                                       from_user_id,
                                       "error||Couldn't find the specified user for pinging.",
                                       52,
                                       0,
                                       0);
                    }
                    else
                    {
                      /* if pinging is requested, */
                      /* we mark the user and inform the sender about this */
                      if (0 == ping_user->ping_status)
                      {
                        ping_user->ping_status = 1;
                        pthread_cond_signal (&ping_user->wake_up_sender);
                      }
                    }
                    pthread_mutex_unlock (&chat_mutex);
                  }
                  else
                  {
                    chat_addmessage (0,
                                     from_user_id,
                                     "error||Error while pinging.",
                                     27,
                                     0,
                                     1);
                  }
                }
                break;

              default:
                /* invalid command */
                chat_addmessage (0,
                                 from_user_id,
                                 "error||You sent an invalid command.",
                                 35,
                                 0,
                                 1);
                break;
              }
            }
          }
          MHD_websocket_free (cu->ws,
                              frame_data);
          return 0;

        case MHD_WEBSOCKET_STATUS_CLOSE_FRAME:
          /* if we receive a close frame, we will respond with one */
          MHD_websocket_free (cu->ws,
                              frame_data);
          {
            char *result = NULL;
            size_t result_len = 0;
            int er = MHD_websocket_encode_close (cu->ws,
                                                 MHD_WEBSOCKET_CLOSEREASON_REGULAR,
                                                 NULL,
                                                 0,
                                                 &result,
                                                 &result_len);
            if (MHD_WEBSOCKET_STATUS_OK == er)
            {
              send_all (cu,
                        result,
                        result_len);
              MHD_websocket_free (cu->ws, result);
            }
          }
          return 1;

        case MHD_WEBSOCKET_STATUS_PING_FRAME:
          /* if we receive a ping frame, we will respond */
          /* with the corresponding pong frame */
          {
            char *pong = NULL;
            size_t pong_len = 0;
            int er = MHD_websocket_encode_pong (cu->ws,
                                                frame_data,
                                                frame_len,
                                                &pong,
                                                &pong_len);

            MHD_websocket_free (cu->ws,
                                frame_data);
            if (MHD_WEBSOCKET_STATUS_OK == er)
            {
              send_all (cu,
                        pong,
                        pong_len);
              MHD_websocket_free (cu->ws,
                                  pong);
            }
          }
          return 0;

        case MHD_WEBSOCKET_STATUS_PONG_FRAME:
          /* if we receive a pong frame, */
          /* we will check whether we requested this frame and */
          /* whether it is the last requested pong */
          if (2 == cu->ping_status)
          {
            cu->ping_status = 0;
            struct timespec now;
            timespec_get (&now, TIME_UTC);
            if ((cu->ping_message_len == frame_len) &&
                (0 == strcmp (frame_data,
                              cu->ping_message)))
            {
              int ping = (int) (((int64_t) (now.tv_sec
                                            - cu->ping_start.tv_sec))  * 1000
                                + ((int64_t) (now.tv_nsec
                                              - cu->ping_start.tv_nsec))
                                / 1000000);
              char result_text[240];
              strcpy (result_text,
                      "ping|");
              snprintf (result_text + 5, 235, "%d", (int) cu->user_id);
              strcat (result_text,
                      "|");
              snprintf (result_text + strlen (result_text), 240 - strlen (
                          result_text), "%d", (int) ping);
              chat_addmessage (0,
                               0,
                               result_text,
                               strlen (result_text),
                               0,
                               1);
            }
          }
          MHD_websocket_free (cu->ws,
                              frame_data);
          return 0;

        default:
          /* This case should really never happen, */
          /* because there are only five types of (finished) websocket frames. */
          /* If it is ever reached, it means that there is memory corruption. */
          MHD_websocket_free (cu->ws,
                              frame_data);
          return 1;
        }
      }
    }
  }

  return 0;
}


/**
 * Sends messages from the message list over the TCP/IP socket
 * after encoding it with the websocket stream.
 * This is also used for server-side actions,
 * because the thread for receiving messages waits for
 * incoming data and cannot be woken up.
 * But the sender thread can be woken up easily.
 *
 * @param cls          The connected user
 * @return             Always NULL
 */
static void *
connecteduser_send_messages (void *cls)
{
  struct ConnectedUser *cu = cls;

  /* the main loop of sending messages requires to lock the mutex */
  if (0 == pthread_mutex_lock (&chat_mutex))
  {
    for (;;)
    {
      /* loop while not all messages processed */
      int all_messages_read = 0;
      while (0 == all_messages_read)
      {
        if (1 == disconnect_all)
        {
          /* the application closes and want that we disconnect all users */
          struct MHD_UpgradeResponseHandle *urh = cu->urh;
          if (NULL != urh)
          {
            /* Close the TCP/IP socket. */
            /* This will also wake-up the waiting receive-thread for this connected user. */
            cu->urh = NULL;
            MHD_upgrade_action (urh,
                                MHD_UPGRADE_ACTION_CLOSE);
          }
          pthread_mutex_unlock (&chat_mutex);
          return NULL;
        }
        else if (1 == cu->disconnect)
        {
          /* The sender thread shall close. */
          /* This is only requested by the receive thread, so we can just leave. */
          pthread_mutex_unlock (&chat_mutex);
          return NULL;
        }
        else if (1 == cu->ping_status)
        {
          /* A pending ping is requested */
          ++cu->ping_counter;
          strcpy (cu->ping_message,
                  "libmicrohttpdchatserverpingdata");
          snprintf (cu->ping_message + 31, 97, "%d", (int) cu->ping_counter);
          cu->ping_message_len = strlen (cu->ping_message);
          char *frame_data = NULL;
          size_t frame_len = 0;
          int er = MHD_websocket_encode_ping (cu->ws,
                                              cu->ping_message,
                                              cu->ping_message_len,
                                              &frame_data,
                                              &frame_len);
          if (MHD_WEBSOCKET_STATUS_OK == er)
          {
            cu->ping_status = 2;
            timespec_get (&cu->ping_start, TIME_UTC);

            /* send the data via the TCP/IP socket and */
            /* unlock the mutex while sending */
            pthread_mutex_unlock (&chat_mutex);
            send_all (cu,
                      frame_data,
                      frame_len);
            if (0 != pthread_mutex_lock (&chat_mutex))
            {
              return NULL;
            }
          }
          MHD_websocket_free (cu->ws, frame_data);
        }
        else if (cu->next_message_index < message_count)
        {
          /* a chat message or command is pending */
          char *frame_data = NULL;
          size_t frame_len = 0;
          int er = 0;
          {
            struct Message *msg = messages[cu->next_message_index];
            if ((0 == msg->to_user_id) ||
                (cu->user_id == msg->to_user_id) ||
                (cu->user_id == msg->from_user_id) )
            {
              if (0 == msg->is_binary)
              {
                er = MHD_websocket_encode_text (cu->ws,
                                                msg->data,
                                                msg->data_len,
                                                MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                                &frame_data,
                                                &frame_len,
                                                NULL);
              }
              else
              {
                er = MHD_websocket_encode_binary (cu->ws,
                                                  msg->data,
                                                  msg->data_len,
                                                  MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                                  &frame_data,
                                                  &frame_len);
              }
            }
          }
          ++cu->next_message_index;

          /* send the data via the TCP/IP socket and */
          /* unlock the mutex while sending */
          pthread_mutex_unlock (&chat_mutex);
          if (MHD_WEBSOCKET_STATUS_OK == er)
          {
            send_all (cu,
                      frame_data,
                      frame_len);
          }
          MHD_websocket_free (cu->ws,
                              frame_data);
          if (0 != pthread_mutex_lock (&chat_mutex))
          {
            return NULL;
          }
          /* check whether there are still pending messages */
          all_messages_read = (cu->next_message_index < message_count) ? 0 : 1;
        }
        else
        {
          all_messages_read = 1;
        }
      }
      /* clear old messages */
      chat_clearmessages (0);

      /* Wait for wake up. */
      /* This will automatically unlock the mutex while waiting and */
      /* lock the mutex after waiting */
      pthread_cond_wait (&cu->wake_up_sender, &chat_mutex);
    }
  }

  return NULL;
}


/**
 * Receives messages from the TCP/IP socket and
 * initializes the connected user.
 *
 * @param cls The connected user
 * @return    Always NULL
 */
static void *
connecteduser_receive_messages (void *cls)
{
  struct ConnectedUser *cu = cls;
  char buf[128];
  ssize_t got;
  int result;

  /* make the socket blocking */
  make_blocking (cu->fd);

  /* generate the user name */
  {
    char user_name[32];
    strcpy (user_name, "User");
    snprintf (user_name + 4, 28, "%d", (int) cu->user_id);
    cu->user_name_len = strlen (user_name);
    cu->user_name = malloc (cu->user_name_len + 1);
    if (NULL == cu->user_name)
    {
      free (cu->extra_in);
      free (cu);
      MHD_upgrade_action (cu->urh,
                          MHD_UPGRADE_ACTION_CLOSE);
      return NULL;
    }
    strcpy (cu->user_name, user_name);
  }

  /* initialize the wake-up-sender condition variable */
  if (0 != pthread_cond_init (&cu->wake_up_sender, NULL))
  {
    MHD_upgrade_action (cu->urh,
                        MHD_UPGRADE_ACTION_CLOSE);
    free (cu->user_name);
    free (cu->extra_in);
    free (cu);
    return NULL;
  }

  /* initialize the send mutex */
  if (0 != pthread_mutex_init (&cu->send_mutex, NULL))
  {
    MHD_upgrade_action (cu->urh,
                        MHD_UPGRADE_ACTION_CLOSE);
    pthread_cond_destroy (&cu->wake_up_sender);
    free (cu->user_name);
    free (cu->extra_in);
    free (cu);
    return NULL;
  }

  /* add the user to the chat user list */
  chat_adduser (cu);

  /* initialize the web socket stream for encoding/decoding */
  result = MHD_websocket_stream_init (&cu->ws,
                                      MHD_WEBSOCKET_FLAG_SERVER
                                      | MHD_WEBSOCKET_FLAG_NO_FRAGMENTS,
                                      0);
  if (MHD_WEBSOCKET_STATUS_OK != result)
  {
    chat_removeuser (cu);
    pthread_cond_destroy (&cu->wake_up_sender);
    pthread_mutex_destroy (&cu->send_mutex);
    MHD_upgrade_action (cu->urh,
                        MHD_UPGRADE_ACTION_CLOSE);
    free (cu->user_name);
    free (cu->extra_in);
    free (cu);
    return NULL;
  }

  /* send a list of all currently connected users (bypassing the messaging system) */
  {
    struct UserInit
    {
      char *user_init;
      size_t user_init_len;
    };
    struct UserInit *init_users = NULL;
    size_t init_users_len = 0;

    /* first collect all users without sending (so the mutex isn't locked too long) */
    if (0 == pthread_mutex_lock (&chat_mutex))
    {
      if (0 < user_count)
      {
        init_users = (struct UserInit *) malloc (user_count * sizeof (struct
                                                                      UserInit));
        if (NULL != init_users)
        {
          init_users_len = user_count;
          for (size_t i = 0; i < user_count; ++i)
          {
            char user_index[32];
            snprintf (user_index, 32, "%d", (int) users[i]->user_id);
            size_t user_index_len = strlen (user_index);
            struct UserInit iu;
            iu.user_init_len = user_index_len + users[i]->user_name_len + 10;
            iu.user_init = (char *) malloc (iu.user_init_len + 1);
            if (NULL != iu.user_init)
            {
              strcpy (iu.user_init, "userinit|");
              strcat (iu.user_init, user_index);
              strcat (iu.user_init, "|");
              if (0 < users[i]->user_name_len)
                strcat (iu.user_init, users[i]->user_name);
            }
            init_users[i] = iu;
          }
        }
      }
      pthread_mutex_unlock (&chat_mutex);
    }

    /* then send all users to the connected client */
    for (size_t i = 0; i < init_users_len; ++i)
    {
      char *frame_data = NULL;
      size_t frame_len = 0;
      if ((0 < init_users[i].user_init_len) && (NULL !=
                                                init_users[i].user_init) )
      {
        int status = MHD_websocket_encode_text (cu->ws,
                                                init_users[i].user_init,
                                                init_users[i].user_init_len,
                                                MHD_WEBSOCKET_FRAGMENTATION_NONE,
                                                &frame_data,
                                                &frame_len,
                                                NULL);
        if (MHD_WEBSOCKET_STATUS_OK == status)
        {
          send_all (cu,
                    frame_data,
                    frame_len);
          MHD_websocket_free (cu->ws,
                              frame_data);
        }
        free (init_users[i].user_init);
      }
    }
    free (init_users);
  }

  /* send the welcome message to the user (bypassing the messaging system) */
  {
    char *frame_data = NULL;
    size_t frame_len = 0;
    const char *welcome_msg = "moderator||" \
                              "Welcome to the libmicrohttpd WebSocket chatserver example.\n" \
                              "Supported commands are:\n" \
                              "  /m <user> <text> - sends a private message to the specified user\n" \
                              "  /ping <user> - sends a ping to the specified user\n" \
                              "  /name <name> - changes your name to the specified name\n" \
                              "  /disconnect - disconnects your websocket\n\n" \
                              "All messages, which does not start with a slash, " \
                              "are regular messages and will be sent to the selected user.\n\n" \
                              "Have fun!";
    MHD_websocket_encode_text (cu->ws,
                               welcome_msg,
                               strlen (welcome_msg),
                               MHD_WEBSOCKET_FRAGMENTATION_NONE,
                               &frame_data,
                               &frame_len,
                               NULL);
    send_all (cu,
              frame_data,
              frame_len);
    MHD_websocket_free (cu->ws,
                        frame_data);
  }

  /* start the message-send thread */
  pthread_t pt;
  if (0 != pthread_create (&pt,
                           NULL,
                           &connecteduser_send_messages,
                           cu))
    abort ();

  /* start by parsing extra data MHD may have already read, if any */
  if (0 != cu->extra_in_size)
  {
    if (0 != connecteduser_parse_received_websocket_stream (cu,
                                                            cu->extra_in,
                                                            cu->extra_in_size))
    {
      chat_removeuser (cu);
      if (0 == pthread_mutex_lock (&chat_mutex))
      {
        cu->disconnect = 1;
        pthread_cond_signal (&cu->wake_up_sender);
        pthread_mutex_unlock (&chat_mutex);
        pthread_join (pt, NULL);
      }
      struct MHD_UpgradeResponseHandle *urh = cu->urh;
      if (NULL != urh)
      {
        cu->urh = NULL;
        MHD_upgrade_action (urh,
                            MHD_UPGRADE_ACTION_CLOSE);
      }
      pthread_cond_destroy (&cu->wake_up_sender);
      pthread_mutex_destroy (&cu->send_mutex);
      MHD_websocket_stream_free (cu->ws);
      free (cu->user_name);
      free (cu->extra_in);
      free (cu);
      return NULL;
    }
    free (cu->extra_in);
    cu->extra_in = NULL;
  }

  /* the main loop for receiving data */
  while (1)
  {
    got = recv (cu->fd,
                buf,
                sizeof (buf),
                0);
    if (0 >= got)
    {
      /* the TCP/IP socket has been closed */
      break;
    }
    if (0 < got)
    {
      if (0 != connecteduser_parse_received_websocket_stream (cu, buf,
                                                              (size_t) got))
      {
        /* A websocket protocol error occurred */
        chat_removeuser (cu);
        if (0 == pthread_mutex_lock (&chat_mutex))
        {
          cu->disconnect = 1;
          pthread_cond_signal (&cu->wake_up_sender);
          pthread_mutex_unlock (&chat_mutex);
          pthread_join (pt, NULL);
        }
        struct MHD_UpgradeResponseHandle *urh = cu->urh;
        if (NULL != urh)
        {
          cu->urh = NULL;
          MHD_upgrade_action (urh,
                              MHD_UPGRADE_ACTION_CLOSE);
        }
        pthread_cond_destroy (&cu->wake_up_sender);
        pthread_mutex_destroy (&cu->send_mutex);
        MHD_websocket_stream_free (cu->ws);
        free (cu->user_name);
        free (cu);
        return NULL;
      }
    }
  }

  /* cleanup */
  chat_removeuser (cu);
  if (0 == pthread_mutex_lock (&chat_mutex))
  {
    cu->disconnect = 1;
    pthread_cond_signal (&cu->wake_up_sender);
    pthread_mutex_unlock (&chat_mutex);
    pthread_join (pt, NULL);
  }
  struct MHD_UpgradeResponseHandle *urh = cu->urh;
  if (NULL != urh)
  {
    cu->urh = NULL;
    MHD_upgrade_action (urh,
                        MHD_UPGRADE_ACTION_CLOSE);
  }
  pthread_cond_destroy (&cu->wake_up_sender);
  pthread_mutex_destroy (&cu->send_mutex);
  MHD_websocket_stream_free (cu->ws);
  free (cu->user_name);
  free (cu);

  return NULL;
}


/**
 * Function called after a protocol "upgrade" response was sent
 * successfully and the socket should now be controlled by some
 * protocol other than HTTP.
 *
 * Any data already received on the socket will be made available in
 * @e extra_in.  This can happen if the application sent extra data
 * before MHD send the upgrade response.  The application should
 * treat data from @a extra_in as if it had read it from the socket.
 *
 * Note that the application must not close() @a sock directly,
 * but instead use #MHD_upgrade_action() for special operations
 * on @a sock.
 *
 * Data forwarding to "upgraded" @a sock will be started as soon
 * as this function return.
 *
 * Except when in 'thread-per-connection' mode, implementations
 * of this function should never block (as it will still be called
 * from within the main event loop).
 *
 * @param cls closure, whatever was given to #MHD_create_response_for_upgrade().
 * @param connection original HTTP connection handle,
 *                   giving the function a last chance
 *                   to inspect the original HTTP request
 * @param con_cls last value left in `con_cls` of the `MHD_AccessHandlerCallback`
 * @param extra_in if we happened to have read bytes after the
 *                 HTTP header already (because the client sent
 *                 more than the HTTP header of the request before
 *                 we sent the upgrade response),
 *                 these are the extra bytes already read from @a sock
 *                 by MHD.  The application should treat these as if
 *                 it had read them from @a sock.
 * @param extra_in_size number of bytes in @a extra_in
 * @param sock socket to use for bi-directional communication
 *        with the client.  For HTTPS, this may not be a socket
 *        that is directly connected to the client and thus certain
 *        operations (TCP-specific setsockopt(), getsockopt(), etc.)
 *        may not work as expected (as the socket could be from a
 *        socketpair() or a TCP-loopback).  The application is expected
 *        to perform read()/recv() and write()/send() calls on the socket.
 *        The application may also call shutdown(), but must not call
 *        close() directly.
 * @param urh argument for #MHD_upgrade_action()s on this @a connection.
 *        Applications must eventually use this callback to (indirectly)
 *        perform the close() action on the @a sock.
 */
static void
upgrade_handler (void *cls,
                 struct MHD_Connection *connection,
                 void *con_cls,
                 const char *extra_in,
                 size_t extra_in_size,
                 MHD_socket fd,
                 struct MHD_UpgradeResponseHandle *urh)
{
  struct ConnectedUser *cu;
  pthread_t pt;
  (void) cls;         /* Unused. Silent compiler warning. */
  (void) connection;  /* Unused. Silent compiler warning. */
  (void) con_cls;     /* Unused. Silent compiler warning. */

  /* This callback must return as soon as possible. */

  /* allocate new connected user */
  cu = malloc (sizeof (struct ConnectedUser));
  if (NULL == cu)
    abort ();
  memset (cu, 0, sizeof (struct ConnectedUser));
  if (0 != extra_in_size)
  {
    cu->extra_in = malloc (extra_in_size);
    if (NULL == cu->extra_in)
      abort ();
    memcpy (cu->extra_in,
            extra_in,
            extra_in_size);
  }
  cu->extra_in_size = extra_in_size;
  cu->fd = fd;
  cu->urh = urh;
  cu->user_id = ++unique_user_id;
  cu->user_name = NULL;
  cu->user_name_len = 0;

  /* create thread for the new connected user */
  if (0 != pthread_create (&pt,
                           NULL,
                           &connecteduser_receive_messages,
                           cu))
    abort ();
  pthread_detach (pt);
}


/**
 * Function called by the MHD_daemon when the client tries to access a page.
 *
 * This is used to provide the main page
 * (in this example HTML + CSS + JavaScript is all in the same file)
 * and to initialize a websocket connection.
 * The rules for the initialization of a websocket connection
 * are listed near the URL check of "/ChatServerWebSocket".
 *
 * @param cls closure, whatever was given to #MHD_start_daemon().
 * @param connection The HTTP connection handle
 * @param url The requested URL
 * @param method The request method (typically "GET")
 * @param version The HTTP version
 * @param upload_data Given upload data for POST requests
 * @param upload_data_size The size of the upload data
 * @param ptr A pointer for request specific data
 * @return MHD_YES on success or MHD_NO on error.
 */
static enum MHD_Result
access_handler (void *cls,
                struct MHD_Connection *connection,
                const char *url,
                const char *method,
                const char *version,
                const char *upload_data,
                size_t *upload_data_size,
                void **ptr)
{
  static int aptr;
  struct MHD_Response *response;
  int ret;
  (void) cls;               /* Unused. Silent compiler warning. */
  (void) version;           /* Unused. Silent compiler warning. */
  (void) upload_data;       /* Unused. Silent compiler warning. */
  (void) upload_data_size;  /* Unused. Silent compiler warning. */

  if (0 != strcmp (method, "GET"))
    return MHD_NO;              /* unexpected method */
  if (&aptr != *ptr)
  {
    /* do never respond on first call */
    *ptr = &aptr;
    return MHD_YES;
  }
  *ptr = NULL;                  /* reset when done */
  if (0 == strcmp (url, "/"))
  {
    /* Default page for visiting the server */
    struct MHD_Response *response = MHD_create_response_from_buffer (strlen (
                                                                       PAGE),
                                                                     PAGE,
                                                                     MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection,
                              MHD_HTTP_OK,
                              response);
    MHD_destroy_response (response);
  }
  else if (0 == strcmp (url, "/ChatServerWebSocket"))
  {
    /**
     * The path for the chat has been accessed.
     * For a valid WebSocket request, at least five headers are required:
     * 1. "Host: <name>"
     * 2. "Connection: Upgrade"
     * 3. "Upgrade: websocket"
     * 4. "Sec-WebSocket-Version: 13"
     * 5. "Sec-WebSocket-Key: <base64 encoded value>"
     * Values are compared in a case-insensitive manner.
     * Furthermore it must be a HTTP/1.1 or higher GET request.
     * See: https://tools.ietf.org/html/rfc6455#section-4.2.1
     *
     * To make this example portable we skip the Host check
     */

    char is_valid = 1;
    const char *value = NULL;
    char sec_websocket_accept[29];

    /* check whether an websocket upgrade is requested */
    if (0 != MHD_websocket_check_http_version (version))
    {
      is_valid = 0;
    }
    value = MHD_lookup_connection_value (connection,
                                         MHD_HEADER_KIND,
                                         MHD_HTTP_HEADER_CONNECTION);
    if (0 != MHD_websocket_check_connection_header (value))
    {
      is_valid = 0;
    }
    value = MHD_lookup_connection_value (connection,
                                         MHD_HEADER_KIND,
                                         MHD_HTTP_HEADER_UPGRADE);
    if (0 != MHD_websocket_check_upgrade_header (value))
    {
      is_valid = 0;
    }
    value = MHD_lookup_connection_value (connection,
                                         MHD_HEADER_KIND,
                                         MHD_HTTP_HEADER_SEC_WEBSOCKET_VERSION);
    if (0 != MHD_websocket_check_version_header (value))
    {
      is_valid = 0;
    }
    value = MHD_lookup_connection_value (connection,
                                         MHD_HEADER_KIND,
                                         MHD_HTTP_HEADER_SEC_WEBSOCKET_KEY);
    if (0 != MHD_websocket_create_accept_header (value, sec_websocket_accept))
    {
      is_valid = 0;
    }

    if (1 == is_valid)
    {
      /* create the response for upgrade */
      response = MHD_create_response_for_upgrade (&upgrade_handler,
                                                  NULL);

      /**
       * For the response we need at least the following headers:
       * 1. "Connection: Upgrade"
       * 2. "Upgrade: websocket"
       * 3. "Sec-WebSocket-Accept: <base64value>"
       * The value for Sec-WebSocket-Accept can be generated with MHD_websocket_create_accept_header.
       * It requires the value of the Sec-WebSocket-Key header of the request.
       * See also: https://tools.ietf.org/html/rfc6455#section-4.2.2
       */
      MHD_add_response_header (response,
                               MHD_HTTP_HEADER_UPGRADE,
                               "websocket");
      MHD_add_response_header (response,
                               MHD_HTTP_HEADER_SEC_WEBSOCKET_ACCEPT,
                               sec_websocket_accept);
      ret = MHD_queue_response (connection,
                                MHD_HTTP_SWITCHING_PROTOCOLS,
                                response);
      MHD_destroy_response (response);
    }
    else
    {
      /* return error page */
      struct MHD_Response *response = MHD_create_response_from_buffer (strlen (
                                                                         PAGE_INVALID_WEBSOCKET_REQUEST),
                                                                       PAGE_INVALID_WEBSOCKET_REQUEST,
                                                                       MHD_RESPMEM_PERSISTENT);
      ret = MHD_queue_response (connection,
                                MHD_HTTP_BAD_REQUEST,
                                response);
      MHD_destroy_response (response);
    }
  }
  else
  {
    struct MHD_Response *response = MHD_create_response_from_buffer (strlen (
                                                                       PAGE_NOT_FOUND),
                                                                     PAGE_NOT_FOUND,
                                                                     MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response (connection,
                              MHD_HTTP_NOT_FOUND,
                              response);
    MHD_destroy_response (response);
  }
  return ret;
}


/**
 * The main routine for this example
 *
 * This starts the daemon and waits for a key hit.
 * After this it will shutdown the daemon.
 */
int
main (int argc,
      char *const *argv)
{
  (void) argc;               /* Unused. Silent compiler warning. */
  (void) argv;               /* Unused. Silent compiler warning. */
  struct MHD_Daemon *d;

  if (0 != pthread_mutex_init (&chat_mutex, NULL))
    return 1;

#if USE_HTTPS == 1
  const char private_key[] = "TODO: Enter your key in PEM format here";
  const char certificate[] = "TODO: Enter your certificate in PEM format here";
  d = MHD_start_daemon (MHD_ALLOW_UPGRADE | MHD_USE_AUTO
                        | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG
                        | MHD_USE_TLS,
                        443,
                        NULL, NULL,
                        &access_handler, NULL,
                        MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
                        MHD_OPTION_HTTPS_MEM_KEY, private_key,
                        MHD_OPTION_HTTPS_MEM_CERT, certificate,
                        MHD_OPTION_END);
#else
  d = MHD_start_daemon (MHD_ALLOW_UPGRADE | MHD_USE_AUTO
                        | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
                        80,
                        NULL, NULL,
                        &access_handler, NULL,
                        MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
                        MHD_OPTION_END);
#endif

  if (NULL == d)
    return 1;
  (void) getc (stdin);

  if (0 == pthread_mutex_lock (&chat_mutex))
  {
    disconnect_all = 1;
    for (size_t i = 0; i < user_count; ++i)
      pthread_cond_signal (&users[i]->wake_up_sender);
    pthread_mutex_unlock (&chat_mutex);
  }
  sleep (2);
  if (0 == pthread_mutex_lock (&chat_mutex))
  {
    for (size_t i = 0; i < user_count; ++i)
    {
      struct MHD_UpgradeResponseHandle *urh = users[i]->urh;
      if (NULL != urh)
      {
        users[i]->urh = NULL;
        MHD_upgrade_action (users[i]->urh,
                            MHD_UPGRADE_ACTION_CLOSE);
      }
    }
    pthread_mutex_unlock (&chat_mutex);
  }
  sleep (2);

  /* usually we should wait here in a safe way for all threads to disconnect, */
  /* but we skip this in the example */

  pthread_mutex_destroy (&chat_mutex);

  MHD_stop_daemon (d);

  return 0;
}
