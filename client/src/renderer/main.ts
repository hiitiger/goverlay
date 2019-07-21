import { ipcRenderer } from "electron"

const startButton = document.getElementById("start") as HTMLButtonElement
startButton.addEventListener("click", () => {
  ipcRenderer.send("start")
})

const injectButton = document.getElementById("inject") as HTMLButtonElement
const titleInput = document.getElementById("title") as HTMLInputElement
injectButton.addEventListener("click", () => {
  const title = titleInput.value
  ipcRenderer.send("inject", title)
})

// const canvas = document.getElementById("canvas") as HTMLCanvasElement
// const context = canvas.getContext("2d")!

const imageElem = document.getElementById("image") as HTMLImageElement

ipcRenderer.on("osrImage", (event: string, arg: { image: string }) => {
  const { image } = arg
  // imageElem.onload = function() {
  //   context.clearRect(0, 0, canvas.width, canvas.height)
  //   context.drawImage(
  //     imageElem,
  //     0,
  //     0,
  //     imageElem.width,
  //     imageElem.height,
  //     0,
  //     0,
  //     canvas.width,
  //     canvas.height
  //   )
  // }
  imageElem.src = image
})

window.onfocus = function() {
  console.log("focus")
}
window.onblur = function() {
  console.log("blur")
}
