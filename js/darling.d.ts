import { BrowserWindow, BrowserWindowConstructorOptions } from 'electron';
import { EventEmitter } from 'events';

export type DarlingCornerPreference = 0 | 1 | 2 | 3;

export interface DarlingWindowOptions {
    // Window dimensions
    width?: number;
    height?: number;
    
    // Window position
    x?: number;
    y?: number;
    center?: boolean;
    
    // Content
    url?: string;
    title?: string;
    
    // Appearance
    showIcon?: boolean;
    frameRate?: number;
    theme?: {
        titlebar?: 'dark' | 'light';
        content?: 'dark' | 'light';
    } | 'dark' | 'light';
    
    // Win32 Native styles 
    nativeStylesAdd?: number;
    nativeStylesRemove?: number;
    nativeExStylesAdd?: number;
    nativeExStylesRemove?: number;
    
    // Electron options
    webPreferences?: BrowserWindowConstructorOptions['webPreferences'];
    
    // Callbacks
    onClose?: () => void;
    onReady?: (window: DarlingWindowInstance) => void;
    onError?: (error: Error) => void;
    electron?: (browserWindow: BrowserWindow, instance: DarlingWindowInstance) => void;
}

export interface DarlingWindowInstance extends EventEmitter {
    // Properties
    readonly darlingWindow: any;
    readonly browserWindow: BrowserWindow;
    readonly handle: any;
    readonly closed: boolean;
    readonly isDestroyed: boolean;
    readonly webContents: Electron.WebContents;
    
    // Methods
    close(): void;
    focus(): void;
    blur(): void;
    resize(width: number, height: number): void;
    move(x: number, y: number): void;
    center(): void;
    setTitle(title: string): void;
    setDarkMode(enabled: boolean): void;
    showWindow(): void;
    hideWindow(): void;
    focusWindow(): void;
    isVisible(): boolean;
    isFocused(): boolean;
    setWindowOpacity(opacity: number): void;
    setAlwaysOnTop(enable: boolean): void;
    setTitlebarColor(color: number): void;
    setCornerPreference(preference: DarlingCornerPreference): void;
    flashWindow(continuous?: boolean): void;
    getDpi(): number;
    getScaleFactor(): number;
    minimize(): void;
    maximize(): void;
    restore(): void;
    
    // Events
    on(event: 'close', listener: () => void): this;
    on(event: 'closed', listener: () => void): this;
    on(event: 'ready', listener: () => void): this;
    on(event: 'resize', listener: (width: number, height: number) => void): this;
    on(event: 'move', listener: (x: number, y: number) => void): this;
    on(event: 'focus', listener: () => void): this;
    on(event: 'blur', listener: () => void): this;
}

export function CreateWindow(options?: DarlingWindowOptions): Promise<DarlingWindowInstance>;
export function GetMainWindow(): DarlingWindowInstance | null;

export default CreateWindow;
