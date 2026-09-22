# WXClient

A C++ client for the ICT Protege WX access controller, using its HTTP API.

## Status

Login, session management, and the controller settings query work. The CLI's
main menu is fully wired: submenu navigation (Doors/Areas/Outputs/Inputs/
Trouble Inputs) lists live controller data, Backup downloads and saves the
programming database, Restart sends the restart command, Logout exits. Acting
on a selected item (`cliCommandMenu`) is not implemented yet — picking one
from a submenu currently does nothing.

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
After login, the main menu offers the Doors/Areas/Outputs/Inputs/Trouble Inputs submenus, plus Backup, Restart, and Logout.

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

## Menus

`core/menu_tables.h` holds only frontend-agnostic protocol data — no CLI
concepts, no selection keys. Anything CLI-specific (the numbered `key`, the
top-level `mainMenu`) lives in `session.cpp`, since a GUI would select by
click and wouldn't want either. Submenu names and `RecId`s come from one
`List` request per table, cached for the session; live status isn't
fetched yet.

## Troubleshooting

Failures are appended to `logs.csv`; for `sendRequest`/`sendCommand`/
`downloadBackup`, `wx.lastError()` holds the same message.

| Response                               | Meaning                                                       |
|----------------------------------------|---------------------------------------------------------------|
| `FAIL`                                 | wrong username or password                                    |
| `FAIL 5` / `FAIL 60`                   | wrong credentials, repeated attempts — wait that many seconds |
| `FAIL. No valid operator login found.` | controller still on default `admin:admin`                     |

| Message                                     | Cause                                                      |
|---------------------------------------------|------------------------------------------------------------|
| `Could not reach controller: Connection`    | wrong `Https?` answer, or nothing listening on that port   |
| `Could not reach controller: SSLConnection` | answered HTTPS, TLS handshake failed                       |
| `Could not reach controller: Read`/`Write`  | connected, then timed out (5s, set in `createClient()`)    |
| `Controller returned HTTP <status>`         | transport fine, request form rejected                      |
| `Controller returned HTTP 301`/`302`        | redirect, not followed — usually a middlebox forcing HTTPS |
| `Failed to finalize decryption`             | session key mismatch                                       |

Isolate transport from protocol with a raw `InitSession` call:

```sh
curl -vk "<http|https>://<IP>/PRT_CTRL_DIN_ISAPI.dll?Command&Type=Session&SubType=InitSession"
```

A bare number in the body means transport and request form are correct.

## Roadmap

- `cliCommandMenu`: acting on a selected submenu item — currently a stub.
- Reverse proxy support: planned, needs `isHttps` split into a client
  transport flag and a controller session-scheme flag (see Network path).
- Live status refresh, further queries beyond `GXT_CONTROLLERSETTINGS_TBL`.
- Hardware verification of the pre-4.00.1676 flow — implemented, untested.
- Windows: `console.cpp` is POSIX-only by design; a GUI would share only
  `core/`, where `logger.cpp` needs a Windows-specific log path.