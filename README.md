# WXClient

A C++ client for the ICT Protege WX access controller, using its HTTP API.

## Status

Authentication, session management, and the first data query are implemented.
`main.cpp` logs in, fetches the controller settings table, prints it, and closes
the session. Both firmware authentication flows are supported and the client
selects between them automatically. See [Roadmap](#roadmap).

## Requirements

| Dependency | Version | Notes |
| --- | --- | --- |
| CMake | 4.4+ | as pinned in `CMakeLists.txt` |
| C++ compiler | C++20 | uses `std::ranges`, `starts_with`, `std::from_chars` |
| OpenSSL | any recent | resolved via `find_package(OpenSSL REQUIRED)` |
| cpp-httplib | 0.53.1 | **vendored** at `include/httplib.h`, no download needed |
| POSIX terminal | — | `console.cpp` uses `termios`; Linux and macOS only |
| Network path | direct | no reverse proxy or TLS-terminating gateway in between — see [Network path](#network-path) |

`CPPHTTPLIB_OPENSSL_SUPPORT` is defined in `CMakeLists.txt`. Without it,
httplib rejects `https://` URLs at runtime rather than at compile time.

## Network path

The client must reach the controller directly. A reverse proxy, TLS-terminating
gateway, or anything else that rewrites the request is not supported.

The `Https?` answer selects the transport *and* the session mode: over HTTP the
controller expects `CheckPassword` and AES-encrypted parameters, over HTTPS it
expects `CheckPasswordServer` and plain text — see
[How authentication works](#how-authentication-works). A single answer cannot
describe two different hops, so a gateway that accepts HTTPS and forwards HTTP
leaves the client and the controller in different modes, and login fails. Three
narrower assumptions are also baked in: only the first `Set-Cookie` header of a
response is kept, redirects are not followed, and over HTTP the response body
must arrive as unmodified hex or decryption fails.

A proxy that re-encrypts to the controller — so the controller still sees HTTPS
— and adds no cookies of its own does satisfy all four assumptions, but is
untested.

## Controller firmware

Two authentication flows exist, and the client selects automatically:

- **Firmware 4.00.1676 or higher** — `InitSession` succeeds as-is, and the
session is identified solely by the cookie from `Set-Cookie`.
- **Firmware before 4.00.1676** — `InitSession` returns `FAIL`, so the client
retries once with a random 32-character hex session ID of its own. That ID is
appended to `InitSession` and `CheckPassword` as `&SessionID=`, an
incrementing `&Sequence=` is appended to every later request, and the ID is
prepended to the request string once logged in.

The retry is announced on stdout and is not an error.

## Building

```sh
cmake -B build
cmake --build build
```

## Running

```sh
./build/WXClient
```

The client prompts for four values, in this order:

```
IP/Domain:   192.168.1.2      # scheme prefix optional; append :port if non-standard
Username:    admin
Password:                     # not echoed
Https? (y/n): y
```

Answer `Https?` to match how the controller actually serves its web interface.
Getting it wrong is the most common cause of a connection failure — see
[Troubleshooting](#troubleshooting). The firmware version is not asked for; it
is detected during login. Invalid `y`/`n` answers re-prompt; end of input
(Ctrl+D) aborts with an error rather than continuing with empty values.

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

On firmware before 4.00.1676 the controller also expects a client-generated
session ID and a monotonic sequence number, added in
`ControllerAPI::buildRequestString`. The session ID is prepended to the
request string *after* encryption, so the controller can read it without the
key.

The session cookie is captured from the first response and replayed on every
subsequent request. cpp-httplib has no cookie jar, so this is handled manually
in `ControllerAPI::sendRequest` — unlike .NET's `HttpClient`, which does
it transparently.

`CloseSession` is issued by `ControllerAPI`'s destructor, so the session is
released on every exit path including exception unwinding.

## Security notes

- **Server certificate verification is disabled** in `createClient()`, matching
  the vendor sample. Controllers ship self-signed certificates. This means HTTPS
  mode is not protected against an active man-in-the-middle. Pin the
  controller's certificate before treating HTTPS mode as trusted.
- Over HTTP the AES session key is derived from the password hash, and the 16
  key bytes are hex *characters* rather than decoded bytes — about 64 bits of
  entropy in a 128-bit key. That is what the firmware expects. It protects
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
| `Controller returned HTTP 301` / `302` | A redirect, which the client does not follow. Usually a middlebox forcing HTTP to HTTPS — connect to the controller directly. |
| `Unexpected session ID response` | A session request returned something other than a number — check the raw response quoted in the message. |
| `Failed to finalize decryption` | `decrypt()` could not unpad the reply — usually a session key mismatch. |
| `Error in getting controller settings: FAIL…` | Logged in, but the query was rejected. Printed rather than thrown; an empty table is returned. |

`InitSession` needs neither authentication nor encryption, so it can be tested
directly to isolate transport problems from protocol ones:

```sh
curl -vk "[http/https]://[IP_ADDRESS]/PRT_CTRL_DIN_ISAPI.dll?Command&Type=Session&SubType=InitSession"
```

A bare number in the response body means the transport and request form are
correct. `-v` also shows the `Set-Cookie` header.

## Layout

```
include/
  ControllerAPI.h    controller client interface
  console.h          terminal prompts, non-echoing password entry, table output
  helpers.h          hex conversion, trimming, query-string parsing, URL decoding
  httplib.h          vendored cpp-httplib 0.53.1
src/
  main.cpp           interactive entry point
  ControllerAPI.cpp  authentication, session, AES payload handling
  console.cpp        terminal input
  helpers.cpp        shared helpers
```

## Roadmap

- Further data queries beyond `GXT_CONTROLLERSETTINGS_TBL` — events, doors, users.
- Windows support: `console.cpp` depends on `termios`.
- Hardware verification of the pre-4.00.1676 flow; the session-ID and
  sequence-number handling is implemented but untested.
- Reverse proxy support is not planned at this time.