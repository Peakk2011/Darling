import { CreateWindow } from 'darling';

CreateWindow({
    width: 800,
    height: 600,
    url: 'https://github.com/Peakk2011/',
    frameRate: 60,
    title: '',
    showIcon: false,

    onClose: () => {
        console.log('Darling Electron window closed.');
    }
})