const Overlay = require('./build/Release/electron_overlay');

console.log(Overlay.hello()); // 'world'
console.log(Overlay);

Overlay.start()


Overlay.setEventCallback((data) => {
    console.log(data)
})

Overlay.sendCommand()

Overlay.sendFrameBuffer(10, "haha")