# WXClient

A C++ client for the ICT Protege WX access controller, using its HTTP API.

## Status

Login, session management, and the controller settings query work. The CLI's
main menu is fully wired: submenus (Doors/Areas/Outputs/Inputs/Trouble Inputs)
list live controller data with decoded status, and selecting an item opens a
control menu for it. Backup saves the programming database; Restart Modules and
Restart Controller ask for confirmation first; Logout exits.

Control commands and status decoding are implemented for Doors only; the other
tables show raw status and report "No control commands" until their lookup
tables are filled in.

## Requirements

| Dependency     | Version    | Notes                                                               |
|----------------|------------|---------------------------------------------------------------------|
| CMake          | 4.4+       | pinned in `CMakeLists.txt`                                          |
| C++ compiler   | C++20      | `std::ranges`, `starts_with`, `std::from_chars`                     |
| OpenSSL        | any recent | via `find_package(OpenSSL REQUIRED)`                                |
| cpp-httplib    | 0.53.1     | vendored at `include/vendor/httplib/httplib.h`                      |
| POSIX terminal | —          | `console.cpp` uses `termios`; Linux and macOS only                  |
| Network path   | direct     | reverse proxy not yet supported — see [Network path](#network-path) |

`CPPHTTPLIB_OPENSSL_SUPPORT` is set in `CMakeLists.txt`; without it httplib
rejects `https://` URLs at runtime rather than at compile time.

## Building and running

```sh
cmake -B build
cmake --build build
./build/WXClient_CLI
```

Prompts for IP/domain, username, password, and whether the controller (not your connection to it) uses HTTPS
— see [Network path](#network-path) for why that distinction matters.
After login, the main menu offers the Doors/Areas/Outputs/Inputs/Trouble Inputs submenus, plus Backup,
Restart Modules, Restart Controller, and Logout.

## Network path

The client must reach the controller directly today — reverse proxy support
is planned but not yet implemented. `isHttps` currently answers two questions
at once: which scheme the client connects with, and which session mode the
controller is in (HTTP → `CheckPassword` + AES-encrypted parameters; HTTPS →
`CheckPasswordServer` + plain text). A gateway that terminates TLS puts those
two hops in different modes, which the current single flag can't express.

## Authentication

Two firmware flows, selected automatically:

- **4.00.1676+** — session identified solely by the `Set-Cookie` cookie.
- **Earlier** — `InitSession` fails, so the client retries with its own
  random session ID, appended to `InitSession`/`CheckPassword` and prepended
  to every later request once logged in.

Both start with `InitSession` returning a random number, XORed against the
username/password-hash and SHA-1'd. Then:

- **HTTP** — `CheckPassword`; a second random number seeds an AES-128 key
  (first 16 chars of a SHA-1). Parameters travel as `hex(iv) || hex(ciphertext)`.
- **HTTPS** — `CheckPasswordServer`; TLS provides confidentiality, no key is
  derived.

`CloseSession` runs from `ControllerApi`'s destructor, so the session closes
on every exit path, including exceptions.

## Security notes

- Certificate verification is disabled in `createClient()` — controllers ship
  self-signed certs. HTTPS mode is not protected against an active MITM.
- Over HTTP the AES key is 16 hex *characters*, not decoded bytes — roughly
  64 bits of entropy in a 128-bit key. That's the firmware's design, not
  this client's.
- Passwords are never echoed or logged; error messages carry controller
  responses only, never request parameters.

## Layout

`wxclient_core` (a CMake `OBJECT` library) holds everything frontend-agnostic;
each frontend links against it as its own executable, so a GUI never pulls in
`console.cpp`'s terminal code. `WXClient_GUI` builds today but is a stub.

Protocol data (table names, status codes, control commands) lives in
`core/lookup/` as `constexpr` tables transcribed from the vendor docs; menu
keys and the top-level menu stay in the CLI's `session.cpp`.

## Troubleshooting

Failures are appended to `logs.csv` in `$XDG_STATE_HOME/wxclient/`
(default `~/.local/state/wxclient/logs.csv`); the CLI prints this path with
every error. For `sendRequest`/`sendCommand`/`downloadBackup`,
`wx.lastError()` holds the same message.

| Response                               | Meaning                                                       |
|----------------------------------------|---------------------------------------------------------------|
| `FAIL`                                 | wrong username or password                                    |
| `FAIL 5` / `FAIL 60`                   | wrong credentials, repeated attempts — wait that many seconds |
| `FAIL. No valid operator login found.` | controller still on default `admin:admin`                     |

| Message                                         | Cause                                                      |
|-------------------------------------------------|------------------------------------------------------------|
| `Could not reach controller: Connection`        | wrong `Https?` answer, or nothing listening on that port   |
| `Could not reach controller: SSLConnection`     | answered HTTPS, TLS handshake failed                       |
| `Could not reach controller: Read`/`Write`      | connected, then timed out (5s, set in `createClient()`)    |
| `Controller returned HTTP <status>`             | transport fine, request form rejected                      |
| `Controller returned HTTP 301`/`302`            | redirect, not followed — usually a middlebox forcing HTTPS |
| `Failed to finalize decryption`                 | session key mismatch                                       |
| `Command Failed (128) Invalid Command SubType.` | the controller doesn't accept Control for that table       |

Isolate transport from protocol with a raw `InitSession` call:

```sh
curl -vk "<http|https>://<IP>/PRT_CTRL_DIN_ISAPI.dll?Command&Type=Session&SubType=InitSession"
```

A bare number in the body means transport and request form are correct.

## Roadmap

- Reverse proxy support: planned, needs `isHttps` split into a client
  transport flag and a controller session-scheme flag (see Network path).
- Status decoding and control commands for Areas, Outputs, Inputs and Trouble
  Inputs (Doors done).
- Hardware verification of the pre-4.00.1676 flow — implemented, untested.
- Windows: `console.cpp` is POSIX-only by design; a GUI would share only
  `core/`, where `logger.cpp` needs a Windows-specific log path.