import { CreateWindow } from "../index";

const darlingWindow = async () => {
  const win = await CreateWindow({
    width: 640,
    height: 480,
    url: "https://github.com/Peakk2011/",
    title: "Peakk2011",
    showIcon: false,
    onReady: () => {
      console.log("Window ready!");
    },
    onClose: () => {
      console.log("Window closed");
    },
  });

  /*
    When using a control outside of CreateWindow, it won't appear
    immediately when you open the window. Instead, it will appear
    along with the web content you've configured. Therefore, it's
    necessary when you want to make changes. You can use a test
    control like this.
  */

  // Test controls
  // win.setTitle("Peakk2011");
  void win;
};

darlingWindow().catch((error) => {
  console.error("Error:", error);
  process.exit(1);
});