import * as path from "path"
import "./debug"

import { fileUrl } from "./utils"

const CONFIG: any = {}

CONFIG.distDir = path.join(__dirname, "../../")

CONFIG.entryUrl = fileUrl(path.join(CONFIG.distDir, "index/index.html"))

global.CONFIG = CONFIG

export default CONFIG
