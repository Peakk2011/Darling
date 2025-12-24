**Overview**

สรุปสถาปัตยกรรมที่แนะนำ (clean + latency ต่ำ)

HTML / CSS / JS (Renderer)
        │
Electron Renderer (Chromium)
        │
Electron Main (Node.js, ESM)
        │   ← IPC (sync path)
Darling JS API (ESM)
        │
CJS bridge
        │
Native addon (.node)
        │
Darling Core (C)
        │
Win32 (HWND)

Darling ทำแค่:
- สร้าง native window
- expose HWND
- control window flags / resize / transparency
- ไม่ยุ่งเรื่อง rendering

Key concept: Electron ต้อง embed เข้า HWND ของเรา (เราเป็น parent), ไม่ใช่ให้ Darling มาเป็น child ของ BrowserWindow

-----------------------------------------------------------------
วิธีที่ 1 — Recommended: Native reparenting (SetParent + WS_CHILD)

หลักการ
- สร้าง Darling window เป็น top-level window (HWND)
- ใน Electron main: สร้าง `BrowserWindow` แบบปกติ (อาจ `show: false` ชั่วคราว)
- จาก `BrowserWindow.getNativeWindowHandle()` จะได้ Buffer ที่เป็น HWND ของ Chromium window
- เรียก native API: `SetParent(electronHWND, darlingHWND)` เพื่อให้ Chromium window อยู่ภายใต้ HWND ของเรา
- ปรับสไตล์ด้วย `SetWindowLongPtr` เพื่อใส่ `WS_CHILD` และเอา `WS_POPUP|WS_OVERLAPPEDWINDOW` ออก
- จัดตำแหน่ง/ขนาดด้วย `SetWindowPos` ให้ match ขนาดลูกของ Darling

ข้อดี
- latency ต่ำ (ตรงไปตรงมา native calls)
- เต็มสิทธิ์การควบคุมพฤติกรรมหน้าต่าง

ข้อควรระวัง
- ต้องจัดการ input focus, z-order และ clipping (บางกรณี DWM หรือ Aero composition ต้องเอาใจ)
- ต้องปิด/เปิด visibility ตอน reparenting ให้เรียบร้อยเพื่อหลีกเลี่ยง flicker

Pseudo API sequence (Electron main, sync calls)

1) สร้าง Darling window (Darling core)
2) dariHWND = darling.getHWND()
3) bw = new BrowserWindow({ width, height, show:false })
4) electronHWND = bw.getNativeWindowHandle() // Buffer -> uint64
5) darling.setWindowStyles(electronHWND, add = WS_CHILD|WS_VISIBLE, remove = WS_POPUP|WS_CAPTION)
6) darling.setParent(electronHWND, dariHWND)
7) darling.setWindowPos(electronHWND, 0,0,w,h, SWP_SHOWWINDOW)
8) bw.show()

-----------------------------------------------------------------
วิธีที่ 2 — Alternative: DWM thumbnail / mirror (offscreen / composition)

หลักการ
- ใช้ DWM APIs เช่น `DwmRegisterThumbnail` เพื่อ render content ของ Chromium window ไปยัง surface ของ Darling หรือ
- ให้ Electron render offscreen (offscreen rendering / shared texture) แล้วแสดงผลใน Darling surface

ข้อดี
- ลดปัญหา reparenting / focus ที่ซับซ้อน
- เหมาะกับกรณีที่ต้องการ composition level advanced (blur, clip, alpha)

ข้อเสีย
- latency สูงกว่า method 1
- ซับซ้อน: ต้อง copy/mirror frames, manage alpha blending, และ synchronization

เมื่อใช้ method 2 แนะนำเฉพาะเมื่อ method 1 ไม่สามารถใช้ได้ (เช่น security restrictions หรือต้องการ special compositing)

-----------------------------------------------------------------
Native API (synchronous, exposed as N-API / CJS bridge)

สัญญา (recommended signatures — synchronous):

- `uint64_t getHWND()` — คืนค่า HWND ของ Darling window เป็น unsigned 64-bit integer
- `bool setParent(uint64_t childHWND, uint64_t parentHWND)` — call `SetParent`
- `bool setWindowStyles(uint64_t hwnd, uint32_t stylesAdd, uint32_t stylesRemove)` — call `SetWindowLongPtr` to add/remove styles
- `bool setWindowPos(uint64_t hwnd, int x, int y, int w, int h, uint32_t flags)` — call `SetWindowPos`
- `bool showWindow(uint64_t hwnd, int cmdShow)` — call `ShowWindow`

หมายเหตุ: คืนค่า/throw เพื่อให้เรียกแบบ synchronous จาก Electron main ได้ทันที

-----------------------------------------------------------------
Sync IPC pattern (Electron Main ← Darling JS API)

แนวคิด: keep all window-parenting logic in Electron main so itเป็นจุดเดียวที่มีสิทธิ์เรียก native addon และจัดการทั้งสอง HWND

Flow ตัวอย่าง:

1) Electron main สร้าง/เรียก `darling.createWindow()` (Darling JS API)
2) `darling.getHWND()` คืน `HWND` (synchronous)
3) Electron สร้าง `BrowserWindow` แล้ว `getNativeWindowHandle()` เพื่อเอา HWND ของ Chromium
4) Electron เรียก `darling.setWindowStyles()` และ `darling.setParent()` (synchronous)
5) Electron ฟัง event resize ของ Darling (หรือเรียก `darling.on('resize', ...)` ที่ส่งผ่าน IPC) เพื่อ update `SetWindowPos` แบบ sync

ตัวอย่างโค้ด: Electron main (ESM)

```js
import { app, BrowserWindow } from 'electron'
import darling from '../js/darling-bridge.cjs'

app.whenReady().then(() => {
  const dariHWND = darling.getHWND()
  const bw = new BrowserWindow({ width: 800, height: 600, show: false })
  const buf = bw.getNativeWindowHandle()
  const eleHWND = buf.readBigUInt64LE(0)

  // add child style and set parent
  darling.setWindowStyles(Number(eleHWND), 0x40000000 /* WS_CHILD */, 0x80000000 /* WS_POPUP */)
  darling.setParent(Number(eleHWND), Number(dariHWND))
  darling.setWindowPos(Number(eleHWND), 0, 0, 800, 600, 0x0040 /* SWP_SHOWWINDOW */)
  bw.show()
})
```

ข้อแนะนำเพิ่มเติม
- ทดสอบกับ compositor/desktop effects (Aero) เพื่อยืนยันว่า clipping และ input ถูกต้อง
- ถ้าต้องการ transparency ให้ Darling เป็น layered window (WS_EX_LAYERED) และใช้ UpdateLayeredWindow หรือ DWM composition ตามความเหมาะสม
- ใส่ fallback path: ถ้า reparenting ล้มเหลว ให้ประเมินใช้ DWM mirror

-----------------------------------------------------------------
ตัวอย่างการเรียกจาก `darling-bridge.cjs` (CommonJS bridge)

```js
const native = require('../bindings/native_stub')
module.exports = {
  getHWND: () => native.getHWND(),
  setParent: (child, parent) => native.setParent(child, parent),
  setWindowStyles: (hwnd, add, remove) => native.setWindowStyles(hwnd, add, remove),
  setWindowPos: (hwnd,x,y,w,h,flags) => native.setWindowPos(hwnd,x,y,w,h,flags),
  showWindow: (hwnd, cmd) => native.showWindow(hwnd, cmd),
}
```

-----------------------------------------------------------------
สรุป
- ถ้าต้องการ latency ต่ำและความเรียบง่าย ให้ใช้ native reparenting (SetParent + WS_CHILD)
- ถ้าต้องการ high-level composition หรือลดปัญหา focus/ownership ให้พิจารณา DWM thumbnail / offscreen rendering แต่แลกด้วยความซับซ้อนและ latency

ถ้าต้องการ ผมจะสร้างตัวอย่าง Electron main, `darling-bridge.cjs` และ native stub ใน repo (fallback) เพื่อให้เริ่มทดลองทันที
