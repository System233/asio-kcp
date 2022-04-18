// Copyright (c) 2022 github.com/System233
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#ifndef IKCP_HPP
#define IKCP_HPP
#include <ikcp.h>
#include <cstdint>

class ikcp : public ikcpcb
{
public:
  using output_func = int(const char *buf, int len, ikcpcb *kcp, void *user);
  ikcp() = delete;
  ~ikcp() = delete;
  void release()
  {
    return ikcp_release(this);
  }
  void setoutput(output_func output)
  {
    return ikcp_setoutput(this, output);
  }
  int send(void const *buf, size_t len)
  {
    return ikcp_send(this, static_cast<char const *>(buf), int(len));
  }
  int waitsnd()
  {
    return ikcp_waitsnd(this);
  }
  int wndsize(int sndwnd, int rcvwnd)
  {
    return ikcp_wndsize(this, sndwnd, rcvwnd);
  }
  int setmtu(int mtu)
  {
    return ikcp_setmtu(this, mtu);
  }
  template <class... Args>
  void log(int mask, const char *fmt, Args... args)
  {
    ikcp_log(this, mask, fmt, std::forward<Args>(args)...);
  }
  int peeksize()
  {
    return ikcp_peeksize(this);
  }
  int input(void const *buf, size_t len)
  {
    return ikcp_input(this, static_cast<char const *>(buf), long(len));
  }
  int nodelay(int nodelay, int interval, int resend, int nc)
  {
    return ikcp_nodelay(this, nodelay, interval, resend, nc);
  }
  uint32_t check(uint32_t time)
  {
    return ikcp_check(this, time);
  }
  void update(uint32_t time)
  {
    ikcp_update(this, time);
  }
  int recv(void *buf, size_t len)
  {
    return ikcp_recv(this, static_cast<char *>(buf), int(len));
  }
  void flush()
  {
    ikcp_flush(this);
  }
  static ikcp *create(uint32_t conv_id, void *user)
  {
    return static_cast<ikcp *>(ikcp_create(conv_id, user));
  }
  static void allocator(decltype(malloc) malloc, decltype(free) free)
  {
    ikcp_allocator(malloc, free);
  }
  uint32_t getconv()
  {
    return ikcp_getconv(this);
  }
  static uint32_t getconv(void const *data)
  {
    return ikcp_getconv(data);
  }
};

#endif