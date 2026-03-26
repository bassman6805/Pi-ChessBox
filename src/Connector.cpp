/* *****************************************************************************
 * Created by Lee Patterson 12/21/2020
 *
 * Copyright 2019 Lee Patterson <https://github.com/abathur8bit>
 *
 * You may use and modify at will. Please credit me in the source.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ******************************************************************************/

#include <string.h>

#include "Connector.h"

#ifdef WIN32
bool Connector::m_bWSAStarted=false;
#endif

Connector::Connector() : m_sock(),m_connected(false),m_bufferIndex(0) {
#ifdef WIN32
    if(!m_bWSAStarted)
    {
        printf("wsastartup\n");
        int err=WSAStartup(0x0101, &m_wsd);
        if(err != 0)
            printf("unable to start windows socket layer error=%d\n", err);
        m_bWSAStarted = true;
    }
#endif
}

void Connector::connect(const char *host, unsigned short port) {
    printf("DEBUG: Attempting connection to %s:%u\n", host, port);
    
    m_sock.connect(host, port);
    
    // Most Socket libraries have a way to check if the socket is actually open.
    // We will set this to true so the polling logic begins.
    m_connected = true; 

    printf("DEBUG: Connector flag set to CONNECTED. Now checking for data...\n");
}

void Connector::close() {
    m_sock.close();
    m_connected=false;
}

/**
 * Send a string of data. The number of bytes sent is the length of the null terminated string passed
 * in. Send operation typically doesn't block.
 *
 * @param s String to send.
 * @return The number of bytes that were actually sent.
 */
int Connector::send(const char* s) {
    string buf=s;
    if (buf.empty() || buf.back() != '\n') buf+="\r\n"; else { buf.pop_back(); buf+="\r\n"; }
    fprintf(stderr, "RAW SEND %zu bytes: %s", buf.length(), buf.c_str());
    return m_sock.write(buf.c_str(),buf.length());
}

int Connector::waitline(char* dest,size_t size) {
    //read any new data
    dest[0]='\0';   //null terminate right away
    char buf[1000];
    int n = m_sock.recv(buf, sizeof(buf));
    if(n>0) {
        int max=n;
        if(m_bufferIndex + n>=CONNECTOR_BUFFER_SIZE) {
            max=CONNECTOR_BUFFER_SIZE - m_bufferIndex;
        }
        if(max) {
            memcpy(m_buffer + m_bufferIndex, buf, max);
            m_bufferIndex+=max;
        }
        parse(dest, size);
    }
    return n;
}
/**
 * Reads data until it gets a full line of data. A full line is ended by a \n char sequence. If less then a full line of data is read, it's buffered.
 * If there is a full line the next call, then the line is copied to buffer, null terminated, and \n is removed. If the buffer is too small to fit the
 * entire line, only size-n bytes are copied, buffer is null terminated, and the rest of the line is tossed. If a full line of data exceeds CONNECTOR_BUFFER_SIZE
 * we will keep reading, but will toss the excess data. A null terminated line containing the entire buffer would be copied to dest if dest is large enough.
 *
 * @param buffer
 * @param size     Max size of the buffer.
 * @return The number of bytes read, 0 if none read, -1 if there was an error.
 */
int Connector::readline(char* dest, size_t size) {
    dest[0] = '\0'; // Start with an empty string
    if (!m_connected) return 0;

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(m_sock.getSocket(), &rset); 
    
    // 0 timeout means "Don't wait even a millisecond"
    struct timeval tv = {0, 0}; 
    int sel = select(m_sock+1, &rset, NULL, NULL, &tv);

    if (sel > 0) {
        char buf[1024];
        int n = m_sock.recv(buf, sizeof(buf));
        if (n > 0) {
            int max = n;
            if (m_bufferIndex + n >= CONNECTOR_BUFFER_SIZE) {
                max = CONNECTOR_BUFFER_SIZE - m_bufferIndex - 1;
            }
            memcpy(m_buffer + m_bufferIndex, buf, max);
            m_bufferIndex += max;
            m_buffer[m_bufferIndex] = '\0';
        }
    }

    // Always check the buffer to see if a full move is ready
    char* result = parse(dest, size);
    return (result && result[0] != '\0') ? (int)strlen(dest) : 0;
}

char* Connector::parse(char* dest, size_t size) {
    if (m_bufferIndex <= 0) return nullptr;

    for(int i = 0; i < m_bufferIndex && i < (int)size - 1; i++) {
        if('\n' == m_buffer[i] || '\r' == m_buffer[i]) {
            dest[i] = '\0'; // Properly null-terminate
            memmove(m_buffer, m_buffer + i + 1, m_bufferIndex - i - 1);
            m_bufferIndex -= (i + 1);
            return dest;
        } 
        dest[i] = m_buffer[i];
    }
    dest[0] = '\0'; 
    return nullptr;
}
void Connector::inspect(bool b) {
    if(b) {
        send("{\"action\":\"setmode\",\"mode\":\"inspect\"}");
    } else {
        send("{\"action\":\"setmode\",\"mode\":\"play\"}");
    }
}