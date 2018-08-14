import { ipcRenderer } from "electron"

const button = document.getElementById("button") as HTMLButtonElement
button.addEventListener("click", () => {
  ipcRenderer.send("click")
})

const canvas = document.getElementById("canvas") as HTMLCanvasElement
const context = canvas.getContext("2d")!

const imageElem = document.getElementById("image") as HTMLImageElement

ipcRenderer.on("osrImage", (event: string, arg: { image: string }) => {
  const { image } = arg
  imageElem.onload = function() {
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
