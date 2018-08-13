import { ipcRenderer } from "electron"
console.log("renderer...")

const canvas = document.getElementById("canvas") as HTMLCanvasElement
const context = canvas.getContext("2d")!

const imageElem = document.getElementById("image") as HTMLImageElement

ipcRenderer.on("osrImage", (event: string, arg: { image: string }) => {
    console.log("osrImage")
    const { image } = arg
    imageElem.onload = function() {
        console.log(imageElem.width, imageElem.height)

        context.clearRect(0, 0, canvas.width, canvas.height)
        context.drawImage(
            imageElem,
            0,
            0,
            imageElem.width,
            imageElem.height,
            0,
            0,
            canvas.width,
            canvas.height
        )
    }
    imageElem.src = image
})
