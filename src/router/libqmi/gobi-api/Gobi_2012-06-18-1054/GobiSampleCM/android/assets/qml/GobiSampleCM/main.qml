/*===========================================================================
FILE:
   main.qml

DESCRIPTION:
   Graphic meta description for Gobi Sample CM

Copyright (c) 2012, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Code Aurora Forum nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
===========================================================================*/

import QtQuick 1.0

Rectangle
{
   id: mainWindow
   width: 360
   height: 360
   color: "#000000"
   state: windowState

   // This window is not stretched to fullscreen
   // which is what we want to demonstrate for now
   Rectangle
   {
      width: 360
      height: 360
      color: "#ffffff"

      // The box on the left hand side of the screen
      Rectangle
      {
         x: 15
         y: 15
         width: 80
         height: 330
         color: "#ffffff"
         anchors.left: parent.left
         anchors.leftMargin: 15
         anchors.verticalCenter: parent.verticalCenter
         border.width: 2
         border.color: "#000000"

         // The connection stats button
         Rectangle
         {
            id: connectIcon
            width: 60
            height: 70
            color: "#ffffff"
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Image
            {
               width: 50
               height: 50
               anchors.horizontalCenter: parent.horizontalCenter
               anchors.top: parent.top
               anchors.topMargin: 5
               source: "Connect.png"
            }

            Text
            {
               text: "Connection"
               anchors.horizontalCenter: parent.horizontalCenter
               anchors.bottom: parent.bottom
               anchors.bottomMargin: 5
               horizontalAlignment: Text.AlignHCenter
               font.pixelSize: 9
            }

            MouseArea
            {
               id: connectionsButtonArea
               anchors.fill: parent
               onClicked:
               {
                  connectionsButton.Click()
               }
            }
         }

         // The info stats button
         Rectangle
         {
            id: infoIcon
            x: 10
            y: 90
            width: 60
            height: 70
            color: "#ffffff"
            anchors.horizontalCenter: parent.horizontalCenter

            Image
            {
               width: 50
               height: 50
               anchors.horizontalCenter: parent.horizontalCenter
               anchors.top: parent.top
               anchors.topMargin: 5
               source: "Info.png"
            }

            Text
            {
               text: "Information"
               font.pixelSize: 9
               anchors.horizontalCenter: parent.horizontalCenter
               anchors.bottom: parent.bottom
               anchors.bottomMargin: 5
               horizontalAlignment: Text.AlignHCenter
            }

            MouseArea
            {
               id: infosButtonArea
               anchors.fill: parent
               onClicked:
               {
                  infosButton.Click()
               }
            }
         }
      }

      // The "connect" button
      Rectangle
      {
         id: connectionButtonID
         x: 240
         y: 10
         width: 100
         height: 30
         color: "#ffffff"
         border.color: "#000000"
         border.width: 2

         Text
         {
            text: connectButtonText
            font.pixelSize: 16
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
         }

         MouseArea
         {
            anchors.fill: parent
            onClicked:
            {
               connectButton.Click()
            }
         }
      }
   }

   // Map a property ID to its human readable text
   function getName( propID )
   {
      if (propID == "status")
      {
         return "State"
      }

      if (propID == "rssi")
      {
         return "RSSI (db)"
      }

      if (propID == "tech")
      {
         return "Technology"
      }

      if (propID == "rx")
      {
         return "Current RX Rate (bps)"
      }

      if (propID == "tx")
      {
         return "Current TX Rate (bps)"
      }

      if (propID == "maxRx")
      {
         return "Maximum RX Rate (bps)"
      }

      if (propID == "maxTx")
      {
         return "Maximum TX Rate (bps)"
      }

      if (propID == "roam")
      {
         return "Roaming Indicator"
      }

      if (propID == "duration")
      {
         return "Connection Duration"
      }

      if (propID == "lifeDuration")
      {
         return "Life Total Connection Duration"
      }

      if (propID == "lifeRx")
      {
         return "Life Total RX Bytes"
      }

      if (propID == "lifeTx")
      {
         return "Life Total TX Bytes"
      }

      if (propID == "manufact")
      {
         return "Manufacturer"
      }

      if (propID == "model")
      {
         return "Model ID"
      }

      if (propID == "hardware")
      {
         return "Hardware Revision"
      }

      if (propID == "firmware")
      {
         return "Firmware Revision"
      }

      if (propID == "mdn")
      {
         return "MDN"
      }

      if (propID == "min")
      {
         return "MIN"
      }

      if (propID == "esn")
      {
         return "ESN"
      }

      if (propID == "meid")
      {
         return "MEID"
      }

      if (propID == "imei")
      {
         return "IMEI"
      }

      if (propID == "imsi")
      {
         return "IMSI"
      }

      return "Bad PropID"
   }

   // Map a property ID to its variable
   function getValue( propID )
   {
      if (propID == "status")
      {
         return status
      }

      if (propID == "rssi")
      {
         return rssi
      }

      if (propID == "tech")
      {
         return tech
      }

      if (propID == "rx")
      {
         return rx
      }

      if (propID == "tx")
      {
         return tx
      }

      if (propID == "maxRx")
      {
         return maxRx
      }

      if (propID == "maxTx")
      {
         return maxTx
      }

      if (propID == "roam")
      {
         return roam
      }

      if (propID == "duration")
      {
         return duration
      }

      if (propID == "lifeDuration")
      {
         return lifeDuration
      }

      if (propID == "lifeRx")
      {
         return lifeRx
      }

      if (propID == "lifeTx")
      {
         return lifeTx
      }

      if (propID == "manufact")
      {
         return manufact
      }

      if (propID == "model")
      {
         return model
      }

      if (propID == "hardware")
      {
         return hardware
      }

      if (propID == "firmware")
      {
         return firmware
      }

      if (propID == "mdn")
      {
         return mdn
      }

      if (propID == "min")
      {
         return min
      }

      if (propID == "esn")
      {
         return esn
      }

      if (propID == "meid")
      {
         return meid
      }

      if (propID == "imei")
      {
         return imei
      }

      if (propID == "imsi")
      {
         return imsi
      }

      return "Bad PropID"
   }

   // Connection statistics list
   Rectangle
   {
      id: connectionStatistics
      x: 110
      y: 50
      width: 230
      height: 180
      color: "#ffffff"
      border.color: "#000000"
      border.width: 2

      ListView
      {
         anchors.fill: parent
         delegate: Item
         {
            x: 5
            height: 15
            Row
            {
               id: connectionStatistic
               spacing: 10

               // PropID is a unique identification string
               // used to generate a human readable string and
               // link to variable
               property string propID: "unknown"

               Text
               {
                  text: getName( propID )
                  width: 150
                  font.pixelSize: 10
                  horizontalAlignment: Text.AlignLeft
                  font.bold: true
               }

               Text
               {
                  text: getValue( propID )
                  width: 80
                  font.pixelSize: 10
                  horizontalAlignment: Text.AlignLeft
               }
            }
         }

         model: ListModel
         {
            id: connectionStats
            ListElement { propID: "status" }
            ListElement { propID: "rssi" }
            ListElement { propID: "tech" }
            ListElement { propID: "rx" }
            ListElement { propID: "tx" }
            ListElement { propID: "maxRx" }
            ListElement { propID: "maxTx" }
            ListElement { propID: "roam" }
            ListElement { propID: "duration" }
            ListElement { propID: "lifeDuration" }
            ListElement { propID: "lifeRx" }
            ListElement { propID: "lifeTx" }
         }
      }
   }

   // Info statistics list
   Rectangle
   {
      id: infos
      x: 110
      y: 50
      width: 230
      height: 180
      color: "#ffffff"
      border.color: "#000000"
      border.width: 2
      visible: false

      ListView
      {
         anchors.fill: parent
         delegate: Item
         {
            x: 5
            height: 15
            Row
            {
               id: info
               spacing: 10

               // PropID is a unique identification string
               // used to generate a human readable string and
               // link to variable
               property string propID: "unknown"

               Text
               {
                  text: getName( propID )
                  width: 100
                  font.pixelSize: 10
                  horizontalAlignment: Text.AlignLeft
                  font.bold: true
               }

               Text
               {
                  text: getValue( propID )
                  width: 120
                  font.pixelSize: 10
                  horizontalAlignment: Text.AlignLeft
               }
            }
         }

         model: ListModel
         {
            ListElement { propID: "manufact" }
            ListElement { propID: "model" }
            ListElement { propID: "hardware" }
            ListElement { propID: "firmware" }
            ListElement { propID: "mdn" }
            ListElement { propID: "min" }
            ListElement { propID: "esn" }
            ListElement { propID: "meid" }
            ListElement { propID: "imei" }
            ListElement { propID: "imsi" }
         }
      }
   }

   // The close button
   Rectangle
   {
      x: 280
      y: 315
      width: 60
      height: 30
      color: "#ffffff"
      border.color: "#000000"
      border.width: 2

      Text
      {
         anchors.fill: parent
         text: "Close"
         verticalAlignment: Text.AlignVCenter
         horizontalAlignment: Text.AlignHCenter
         font.pixelSize: 16
      }

      MouseArea
      {
         anchors.fill: parent
         onClicked:
         {
            Qt.quit();
         }
      }
   }

   Text
   {
      id: apnTextID
      x: 110
      y: 246
      width: 73
      height: 20
      text: "APN"
      verticalAlignment: Text.AlignVCenter
      horizontalAlignment: Text.AlignRight
      font.pixelSize: 12
   }

   Text
   {
      id: usernameTextID
      x: 110
      y: 266
      width: 73
      height: 20
      text: "Username"
      font.pixelSize: 12
      horizontalAlignment: Text.AlignRight
      verticalAlignment: Text.AlignVCenter
   }

   Text
   {
      id: passwordTextID
      x: 110
      y: 286
      width: 73
      height: 20
      text: "Password"
      font.pixelSize: 12
      horizontalAlignment: Text.AlignRight
      verticalAlignment: Text.AlignVCenter
   }

   // The APN text box
   Rectangle
   {
      id: apnTextBoxID
      x: 190
      y: 246
      width: 150
      height: 20
      color: "#ffffff"
      border.width: 2
      border.color: "#000000"

      TextInput
      {
         anchors.fill: parent
         id: apnName
         font.pixelSize: 12
      }

      Binding
      {
         target: apnNameText
         property: "text"
         value: apnName.text
      }
   }

   // The Username text box
   Rectangle
   {
      id: usernameTextBoxID
      x: 190
      y: 266
      width: 150
      height: 20
      color: "#ffffff"
      border.color: "#000000"
      border.width: 2

      TextInput
      {
         anchors.fill: parent
         id: username
         font.pixelSize: 12
      }

      Binding
      {
         target: usernameText
         property: "text"
         value: username.text
      }
   }

   // Password text box
   Rectangle
   {
      id: passwordTextBoxID
      x: 190
      y: 286
      width: 150
      height: 20
      color: "#ffffff"
      border.color: "#000000"
      border.width: 2

      TextInput
      {
         anchors.fill: parent
         id: password
         font.pixelSize: 12
      }

      Binding
      {
         target: passwordText
         property: "text"
         value: password.text
      }
   }

   // Dialog window to be shown
   Rectangle
   {
      id: connectingDialog
      x: 55
      y: 105
      width: 250
      height: 150
      color: "#ffffff"
      border.color: "#000000"
      border.width: 2
      z: 1
      visible: false

      Text
      {
         id: connectingDialogText
         text: dialogText
         horizontalAlignment: Text.AlignHCenter
         anchors.top: parent.top
         anchors.topMargin: 30
         anchors.horizontalCenter: parent.horizontalCenter
         font.pixelSize: 12
      }
   }

   // State transitions
   states:
   [
      // Info state, shown when info button is clicked
      State
      {
         name: "infos"

         PropertyChanges
         {
            target: connectionButtonID
            visible: false
         }

         PropertyChanges
         {
            target: apnTextID
            visible: false
         }

         PropertyChanges
         {
            target: usernameTextID
            visible: false
         }

         PropertyChanges
         {
            target: passwordTextID
            visible: false
         }

         PropertyChanges
         {
            target: apnTextBoxID
            visible: false
         }

         PropertyChanges
         {
            target: usernameTextBoxID
            visible: false
         }

         PropertyChanges
         {
            target: passwordTextBoxID
            visible: false
         }

         PropertyChanges
         {
            target: connectionStatistics
            visible: false
         }

         PropertyChanges
         {
            target: infos
            visible: true
         }

         PropertyChanges
         {
            target: connectingDialog
            visible: false
         }
      },

      // Connecting Dialog state, shown when connecting
      State
      {
          name: "connectingDialog"

          PropertyChanges
          {
             target: connectingDialog
             visible: true
          }

          // Disable the "info stats" button while the connection dialog is up
          PropertyChanges
          {
             target: infosButtonArea
             visible: false
          }

          // Disable the "connection stats" button while the connection
          // dialog is up
          PropertyChanges
          {
             target: connectionsButtonArea
             visible: false
          }
      }
   ]
}


