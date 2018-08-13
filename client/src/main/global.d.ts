declare namespace NodeJS {
    export interface Global {
        DEBUG: any;
        CONFIG: {
            distDir: string;
            entryUrl: string;
        };
    }
}
