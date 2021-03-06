/*
    Copyright (c) 2007-2015 Contributors as noted in the AUTHORS file

    This file is part of libzmq, the ZeroMQ core engine in C++.

    libzmq is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    As a special exception, the Contributors give you permission to link
    this library with independent modules to produce an executable,
    regardless of the license terms of these independent modules, and to
    copy and distribute the resulting executable under terms of your choice,
    provided that you also meet, for each linked independent module, the
    terms and conditions of the license of that module. An independent
    module is a module which is not derived from or based on this library.
    If you modify this library, you must extend this exception to your
    version of the library.

    libzmq is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "client.hpp"
#include "err.hpp"
#include "msg.hpp"

zmq::client_t::client_t (class ctx_t *parent_, uint32_t tid_, int sid_) :
    socket_base_t (parent_, tid_, sid_, true)
{
    options.type = ZMQ_CLIENT;
}

zmq::client_t::~client_t ()
{
}

void zmq::client_t::xattach_pipe (pipe_t *pipe_, bool subscribe_to_all_)
{
	ZMQ_UNUSED(subscribe_to_all_);

    zmq_assert (pipe_);    

    fq.attach (pipe_);
    lb.attach (pipe_);
}

int zmq::client_t::xsend (msg_t *msg_)
{
    zmq_assert(!(msg_->flags () & msg_t::more));

    return lb.sendpipe (msg_, NULL);
}

int zmq::client_t::xrecv (msg_t *msg_)
{        
    int rc = fq.recvpipe (msg_, NULL);

    // Drop any messages with more flag
    while (rc == 0 && msg_->flags () & msg_t::more) {

        // drop all frames of the current multi-frame message
        rc = fq.recvpipe (msg_, NULL);            
        
        while (rc == 0 && msg_->flags () & msg_t::more)
            rc = fq.recvpipe (msg_, NULL);            

        // get the new message
        if (rc == 0)            
            rc = fq.recvpipe (msg_, NULL);
    }    

    return rc;
}

bool zmq::client_t::xhas_in ()
{
    return fq.has_in ();
}

bool zmq::client_t::xhas_out ()
{
    return lb.has_out ();
}

zmq::blob_t zmq::client_t::get_credential () const
{
    return fq.get_credential ();
}

void zmq::client_t::xread_activated (pipe_t *pipe_)
{
    fq.activated (pipe_);
}

void zmq::client_t::xwrite_activated (pipe_t *pipe_)
{
    lb.activated (pipe_);
}

void zmq::client_t::xpipe_terminated (pipe_t *pipe_)
{
    fq.pipe_terminated (pipe_);
    lb.pipe_terminated (pipe_);
}
