# WXClient

A C++ client for the ICT Protege WX door controller HTTP API, exposed by the
controller as `PRT_CTRL_DIN_ISAPI.dll`.

## Status

Authentication and session management are implemented. Data queries are not yet
— `main.cpp` logs in, reports success, and closes the session. See
[Roadmap](#roadmap).

## Requirements

| Dependency | Version | Notes |
| --- | --- | --- |
| CMake | 4.4+ | as pinned in `CMakeLists.txt` |
| C++ compiler | C++20 | uses `std::ranges`, `starts_with`, `std::from_chars` |
| OpenSSL | any recent | resolved via `find_package(OpenSSL REQUIRED)` |
| cpp-httplib | 0.53.1 | **vendored** at `include/httplib.h`, no download needed |
| POSIX terminal | — | `console.cpp` uses `termios`; Linux and macOS only |

`CPPHTTPLIB_OPENSSL_SUPPORT` is defined in `CMakeLists.txt`. Without it,
httplib rejects `https://` URLs at runtime rather than at compile time.

## Controller firmware

This client implements the **server-side** authentication flow, which requires
**Protege WX firmware 4.00.1676 or higher**. Controllers on older firmware use a
different client-side flow that this client does not implement, and will fail to
authenticate regardless of correct credentials.

## Building

```sh
cmake -B build
cmake --build build
```

## Running

```sh
./build/WXClient
```

The client prompts for four values:

```
IP/Domain:   192.168.1.2      # scheme prefix optional; append :port if non-standard
Https? (y/n): y
Username:    admin
Password:                     # not echoed
```

Answer `Https?` to match how the controller actually serves its web interface.
Getting it wrong is the most common cause of a connection failure — see
[Troubleshooting](#troubleshooting). Invalid `y`/`n` answers re-prompt; end of
input (Ctrl+D) aborts with an error rather than continuing with empty values.

## How authentication works

Both transports begin the same way:

1. `InitSession` — the controller returns a random 32-bit number and a
   `Set-Cookie` header identifying the session.
2. The client XORs the username against `random + 1` and the SHA-1 of the
   password against `random`, then SHA-1s both results.
3. Those two hashes are submitted for verification.

From there the transports diverge:

- **HTTP** — verification uses `CheckPassword`. The controller returns a second
  random number; SHA-1 of `passwordHash XOR random2` provides the first 16
  characters as an AES-128 key. Every subsequent request has its parameters
  encrypted AES-128-CBC and transmitted as `hex(iv) || hex(ciphertext)`.
- **HTTPS** — verification uses `CheckPasswordServer`. TLS provides
  confidentiality, so no AES key is derived and parameters are sent in plain
  text. The second random number is returned but unused.

The session cookie is captured from the first response and replayed on every
subsequent request. cpp-httplib has no cookie jar, so this is handled manually
in `ControllerAPI::getResponseString` — unlike .NET's `HttpClient`, which does
it transparently.

`CloseSession` is issued by `ControllerAPI`'s destructor, so the session is
released on every exit path including exception unwinding.

## Security notes

- **Server certificate verification is disabled** in `createClient()`, matching
  the vendor sample. Controllers ship self-signed certificates. This means HTTPS
  mode is not protected against an active man-in-the-middle. Pin the
  controller's certificate before treating HTTPS mode as trusted.
- Over HTTP the AES session key is derived from the password hash. It protects
  request parameters from casual inspection but is not a substitute for TLS.
- The password is read without terminal echo and is never written to output.
  Error messages carry controller responses only, never request parameters.

## Login errors

The controller returns failures as plain text, all beginning with `FAIL`:

| Response | Meaning |
| --- | --- |
| `FAIL` | wrong username or password |
| `FAIL 5` | wrong credentials, 3+ recent attempts — wait 5 seconds |
| `FAIL 60` | wrong credentials, 6+ recent attempts — wait 60 seconds |
| `FAIL. No valid operator login found.` | controller defaulted; `admin:admin` must be replaced with unique credentials |

The client prints the response verbatim, so the backoff period is visible.

## Troubleshooting

| Message | Cause |
| --- | --- |
| `Could not reach controller: Connection` | Nothing listening on that host and port. Usually the wrong answer to `Https?` — `http://` targets port 80, `https://` targets 443. Also check for a non-standard port. |
| `Could not reach controller: SSLConnection` | Answered `y` but the TLS handshake failed. |
| `Could not reach controller: Read` / `Write` | Connected, then timed out. Timeouts are 5 seconds, set in `createClient()`. |
| `Controller returned HTTP <status>` | The web server answered and rejected the request. The transport is fine; the request form or path is not. |
| `Unexpected session ID response` | A session request returned something other than a number — check the raw response quoted in the message. |

`InitSession` needs neither authentication nor encryption, so it can be tested
directly to isolate transport problems from protocol ones:

```sh
curl -vk "https://192.168.1.2/PRT_CTRL_DIN_ISAPI.dll?Command&Type=Session&SubType=InitSession"
```

A bare number in the response body means the transport and request form are
correct. `-v` also shows the `Set-Cookie` header.

## Layout

```
include/
  ControllerAPI.h    controller client interface
  console.h          terminal prompts and non-echoing password entry
  helpers.h          hex conversion and string trimming
  httplib.h          vendored cpp-httplib 0.53.1
src/
  main.cpp           interactive entry point
  ControllerAPI.cpp  authentication, session, AES payload handling
  console.cpp        terminal input
  helpers.cpp        shared helpers
```

## Roadmap

- Data queries, starting with `Request&Type=Detail&SubType=GXT_CONTROLLERSETTINGS_TBL`
  for controller details such as `SERIALNUMBER`. This is also the first request
  to exercise the AES encrypt/decrypt path, which is currently untested — and
  is bypassed entirely when connecting over HTTPS.
- A parser for the `key=value&...` response format, with percent-decoding.
- Windows support: `console.cpp` depends on `termios`.