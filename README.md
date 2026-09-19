# WXClient

A C++ client for the ICT Protege WX access controller, using its HTTP API.

## Status

Authentication, session management, and the controller settings query work.
`main.cpp` is a thin entry point; `session.cpp` holds the flow — log in, fetch
and print the settings table, then loop on the main menu. Menu navigation is
partial: the top-level menu renders and validates input, but only `Logout` is
wired up; the submenus and their `List`/`Status` queries don't exist yet. Both
firmware authentication flows are supported and selected automatically.

## Requirements

| Dependency     | Version    | Notes                                                |
|----------------|------------|------------------------------------------------------|
| CMake          | 4.4+       | pinned in `CMakeLists.txt`                           |
| C++ compiler   | C++20      | `std::ranges`, `starts_with`, `std::from_chars`      |
| OpenSSL        | any recent | via `find_package(OpenSSL REQUIRED)`                 |
| cpp-httplib    | 0.53.1     | **vendored** at `include/core/httplib.h`             |
| POSIX terminal | —          | `console.cpp` uses `termios`; Linux and macOS only   |
| Network path   | direct     | no reverse proxy — see [Network path](#network-path) |

`CPPHTTPLIB_OPENSSL_SUPPORT` is set in `CMakeLists.txt`; without it httplib
rejects `https://` URLs at runtime rather than at compile time.

## Building and running

```sh
cmake -B build
cmake --build build
./build/WXClient_CLI
```

Four prompts, in order:

```
IP/Domain:    192.168.1.2   # scheme optional; append :port if non-standard
Username:     admin
Password:                   # not echoed
Https? (y/n): y
```

`Https?` must match how the controller actually serves its web interface —
getting it wrong is the most common connection failure. Firmware version is
detected, not asked. Invalid `y`/`n` re-prompts; Ctrl+D aborts rather than
continuing with empty values.

After login the settings table prints, followed by the main menu:

```
1. Doors
2. Areas
3. Outputs
4. Inputs
5. Trouble Inputs
0. Logout

Select:
```

Numbers not listed re-prompt. Only `0` currently does anything.

## Network path

The client must reach the controller directly; a reverse proxy or
TLS-terminating gateway is not supported.

`Https?` selects the transport *and* the session mode (HTTP → `CheckPassword`
plus AES-encrypted parameters; HTTPS → `CheckPasswordServer` plus plain text).
One answer cannot describe two hops, so a gateway that accepts HTTPS and
forwards HTTP leaves the client and controller in different modes and login
fails. Three narrower assumptions also hold: only the first `Set-Cookie` of a
response is kept, redirects are not followed, and over HTTP the body must
arrive as unmodified hex or decryption fails.

A proxy that re-encrypts to the controller and adds no cookies of its own
satisfies all four, but is untested.

## Authentication

Two firmware flows exist, selected automatically:

- **4.00.1676 and later** — `InitSession` succeeds as-is; the session is
  identified solely by the `Set-Cookie` cookie.
- **Earlier** — `InitSession` returns `FAIL`, so the client retries once with
  its own random 32-character hex session ID, appended to `InitSession` and
  `CheckPassword` as `&SessionID=`, with an incrementing `&Sequence=` on every
  later request and the ID prepended to the request string once logged in
  (`Controller_Api::buildRequestString`, applied *after* encryption so the
  controller can read it without the key). The retry is printed to stdout and
  is not an error.

Both transports start the same way: `InitSession` returns a random 32-bit
number; the client XORs the username against `random + 1` and the SHA-1 of the
password against `random`, SHA-1s both results, and submits them. Then they
diverge:

- **HTTP** — verification uses `CheckPassword`. A second random number comes
  back; SHA-1 of `passwordHash XOR random2` supplies the first 16 characters as
  an AES-128 key. Subsequent parameters are AES-128-CBC encrypted and sent as
  `hex(iv) || hex(ciphertext)`.
- **HTTPS** — verification uses `CheckPasswordServer`. TLS provides
  confidentiality, so no key is derived, parameters are plain text, and the
  second random number is unused.

cpp-httplib has no cookie jar, so the session cookie is captured and replayed
manually in `Controller_Api::getResponseString`. `CloseSession` is issued by
`Controller_Api`'s destructor, so the session is released on every exit path
including exception unwinding.

## Security notes

- **Certificate verification is disabled** in `createClient()`, matching the
  vendor sample — controllers ship self-signed certificates. HTTPS mode is
  therefore not protected against an active man-in-the-middle; pin the
  controller's certificate before treating it as trusted.
- Over HTTP the AES key's 16 bytes are hex *characters* rather than decoded
  bytes — roughly 64 bits of entropy in a 128-bit key. That is what the
  firmware expects; it resists casual inspection, not a real attack.
- The password is never echoed or logged. Error messages carry controller
  responses only, never request parameters.

## Menus

**Static menus** are compile-time `constexpr std::array` tables of
`MenuItem{key, label}` in `include/core/menu_tables.h`. The top-level
menu is one; so are the per-entity command lists, whose `key` is the `Command=`
value `sendCommand` takes (fixed by the protocol — see
`docs/vendor/md/control.md`). They live in `core/` because a GUI needs the same
tables. Only `mainMenu` exists so far.

**Dynamic menus** are built at runtime from controller data and carry each
record's `RecId` alongside its label, since a follow-up request or command needs
it. Neither the entry count nor the label text is known until the controller
answers, so they cannot be `constexpr`. Not implemented yet.

The two halves refresh on different schedules. Names and `RecId`s come from one
`List` request per table at login, cached for the session and refreshed only on
demand or after a command changes them. Status is volatile: fetched on entering
a menu and kept in a separate lookup keyed by `RecId`, merged with the label
only at render time, so refreshing status does not rebuild the menu.

`printMenu` renders either kind. It takes a `std::span<const MenuItem>`, prints
`key. label` per row, and re-prompts until the input parses as a number matching
one of the table's keys.

## Troubleshooting

Failures are appended to `logs.csv` (path resolved in `logger.cpp`); for
`sendRequest`/`sendCommand`, `wx.lastError()` holds the same message.

Login failures arrive as plain text beginning with `FAIL`, printed verbatim so
any backoff period is visible:

| Response                               | Meaning                                                             |
|----------------------------------------|---------------------------------------------------------------------|
| `FAIL`                                 | wrong username or password                                          |
| `FAIL 5` / `FAIL 60`                   | wrong credentials, 3+ / 6+ recent attempts — wait that many seconds |
| `FAIL. No valid operator login found.` | controller still on default `admin:admin`; set unique credentials   |

| Message                                                  | Cause                                                                                                                                         |
|----------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------|
| `Could not reach controller: Connection`                 | Nothing listening. Usually the wrong `Https?` answer — `http://` targets port 80, `https://` targets 443. Also check for a non-standard port. |
| `Could not reach controller: SSLConnection`              | Answered `y`, but the TLS handshake failed.                                                                                                   |
| `Could not reach controller: Read` / `Write`             | Connected, then timed out. Timeouts are 5 seconds, set in `createClient()`.                                                                   |
| `Controller returned HTTP <status>`                      | The server answered and rejected the request — transport is fine, the request form is not.                                                    |
| `Controller returned HTTP 301` / `302`                   | A redirect, which the client does not follow. Usually a middlebox forcing HTTPS — connect directly.                                           |
| `Unexpected session ID response`                         | A session request returned something other than a number; the raw response is quoted in the message.                                          |
| `Failed to finalize decryption`                          | `decrypt()` could not unpad the reply — usually a session key mismatch.                                                                       |
| `Could not get controller settings: Request Failed (N)…` | Logged in, but the query was rejected. `sendRequest` returns `std::nullopt`.                                                                  |

`InitSession` needs neither authentication nor encryption, so it isolates
transport problems from protocol ones:

```sh
curl -vk "<http|https>://<IP>/PRT_CTRL_DIN_ISAPI.dll?Command&Type=Session&SubType=InitSession"
```

A bare number in the body means the transport and request form are correct.
`-v` also shows the `Set-Cookie` header.

## Layout

`wxclient_core` (a CMake `OBJECT` library) holds everything frontend-agnostic;
each frontend is its own executable linking against it, so a GUI never links
`console.cpp`'s terminal-specific code. `WXClient_GUI` builds today but is a stub.

```
include/
  core/
    controller_api.h      controller client interface
    helpers.h             hex conversion, trimming, query-string parsing, URL decoding
    logger.h              audit-trail logging to logs.csv
    menu_tables.h  compile-time menu tables (MenuItem, mainMenu)
    workflow.h            shared login+query sequence
    httplib.h             vendored cpp-httplib 0.53.1
  frontend/
    cli/console.h         terminal prompts, password entry, table and menu output
    cli/session.h         runCli()
    gui/session.h         stub
src/
  core/
    Controller_Api.cpp    authentication, session, AES payload handling
    helpers.cpp           shared helpers
    logger.cpp            writes logs.csv to a fixed per-user directory
    workflow.cpp          shared login+query sequence
  frontend/
    cli/main.cpp          entry point — try/catch around runCli()
    cli/session.cpp       login, settings fetch, menu loop
    cli/console.cpp       terminal input, table and menu rendering
    gui/main.cpp          stub entry point
    gui/session.cpp       stub
```

## Roadmap

- Menu dispatch: the top-level keys are raw `int`s shared between
  `static_menu_tables.h` and `session.cpp` with nothing tying them together. An
  `enum class` would let `-Wswitch` catch an unhandled entry.
- Dynamic menus: one `List` request per table at login, cached in `core/` so the
  GUI can reuse it, plus per-menu `Status` requests merged in at render time.
- Live status refresh: manual in the CLI, polling in the GUI. Polling without
  reprinting the whole menu needs in-place terminal redraw.
- Further queries beyond `GXT_CONTROLLERSETTINGS_TBL` — events, doors, users.
- Hardware verification of the pre-4.00.1676 flow; implemented but untested.
- Windows: `console.cpp` is POSIX-only by design, since it is launched via a
  shell script. A GUI frontend would use its toolkit's own input widgets and
  share only `core/`, where `logger.cpp` is the one file needing a
  Windows-specific log path and timestamp call.
- Reverse proxy support is not planned at this time.