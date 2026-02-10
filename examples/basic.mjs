import { CreateWindow } from 'darling';

const darlingWindow = async () => {
    // Create a window
    const win = await CreateWindow({
        width: 640,
        height: 640,
        url: 'https://webpeakkofficial.web.app/',
        title: 'Peakk',
        showIcon: false,

        onReady: () => {
            console.log('Window ready!');
        },

        onClose: () => {
            console.log('Window closed');
        }
    });

    /*
        When using a Control outside of CreateWindow, it won't appear
        immediately when you open the window. Instead, it will appear
        along with the web content you've configured. Therefore, it's
        necessary when you want to make changes. You can use a Test
        Control like this.
    */

    // Test controls
    // win.setTitle('Peakk2011');
}

// Error handler
darlingWindow().catch((error) => {
    console.error('Error:', error);
    process.exit(1);
});