import { app } from 'electron';
import { CreateWindow } from '../js/darling-electron-wrapper.mjs';

app.whenReady().then(() => {
    CreateWindow({
        width: 640,
        height: 480,
        url: 'https://mint-teams.web.app/FascinateNotes/src/',
        frameRate: 60,
        onClose: () => {
            console.log('Darling Electron window closed.');
        }
    })
});

app.on('window-all-closed', () => {
    app.quit();
});