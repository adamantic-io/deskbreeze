/*
 * DeskBreeze IPC JavaScript Interface
 * This file provides the IPC interface for communication between the webview and native code.
 * It is designed to work with the DeskBreeze app's WebView implementation.
 * 
 * This is not directly embedded in the engine so it can be easily used as a
 * reference and as a base for IPC automation on the frontend side. However,
 * it is still tightly coupled with the native code and should not be used
 * outside of the DeskBreeze app (i.e. it will fail if you load it in a
 * regular browser).
 */

if (typeof window?.webkit?.messageHandlers?.deskbreeze === 'undefined') {
    console.error("DeskBreeze IPC is not available. Ensure the native code is properly integrated.");
    throw new Error("DeskBreeze IPC not available");
}

/*
 * Native hook for sending messages to the native code.
 * In webkit this is very convenient; in other platforms, define this to be
 * a function accepting a single argument (the message) and sending it to the native code.
 */ 
const __nativePostMessage = window.webkit.messageHandlers.deskbreeze
                                .postMessage
                                .bind(window.webkit.messageHandlers.deskbreeze);

/*
 * DeskBreeze IPC interface
 * This object provides methods for sending messages to the native code and handling responses.
 * It uses promises to handle asynchronous communication.
 */
window.dbr = {
    ...window.dbr || {},
    ipc: {
        _promises: {},
        _callId: 0,
        send: async function(method, params, timeout=30000) {
            
            const id = (++this._callId).toString(10);
            const p = new Promise((resolve, reject) => {
                this._promises[id] = { resolve, reject };
                setTimeout(() => {
                    if (!(id in this._promises)) {
                        return; // Already resolved or rejected
                    }
                    reject(new Error(`IPC call to ${method} timed out after ${timeout}ms`));
                    delete this._promises[id];

                }, timeout);
            });

            // Send message to native code
            const message = { method: method, params: params, id: id };        
            window.dbr.ipc._sendMessage(message);
            return p;
        },

        _handleResponse: function(response) {
            if (response.id in this._promises) {
                const { resolve, reject } = this._promises[response.id];
                delete this._promises[response.id];
                if (response.error) {
                    reject(new Error(response.error));
                } else {
                    resolve(response.result);
                }
            }
        },
        _handleMessage: function(message) {
            if (typeof message === 'string') {
                try {
                    message = JSON.parse(message);
                } catch (e) {
                    console.error("Failed to parse IPC message:", e);
                    return;
                }
            }
            this.messageHandler(message);
        },
        _sendMessage: function(message) {
            // Convert message to JSON and send to native code
            const jsonMessage = JSON.stringify(message);
            try {
                __nativePostMessage(jsonMessage);
            } catch (e) {
                console.error("Failed to send IPC message:", e);
            }
        },

        messageHandler: function(msg) {
            console.log("*** Default message handler ***", msg);
        }
    }
};

