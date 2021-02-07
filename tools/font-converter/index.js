const font = document.querySelector('.js-font');
const pages = document.querySelector('.js-pages');
const convert = document.querySelector('.js-convert');
const output = document.querySelector('.js-output');

function parseFont() {
    return new Promise((resolve, reject)=> {
        let parsedFont = 'const character_t characters[] = { ';
        if (!font.files[0]) return reject();
        const test = new FileReader();
        test.readAsText(font.files[0]);
        test.onload = ()=> {
            const fontData = (parseBMFontAscii(test.result));
            let characters = '\n';
            const chars = fontData.chars;
            for (let i = 0; i < chars.length; i++) {
                const { page, x, y, width, height, xadvance, yoffset } = chars[i];
                characters += `\t{${page}, ${x}, ${y}, ${width}, ${height}, ${xadvance}, ${yoffset}},\n`;
            }
            characters = characters.slice(0, -2);
            parsedFont += characters + '\n};\n\n';
            fontData.kernings.sort((a, b)=> {
                let diff = a.first - b.first;
                if (diff == 0) return a.second - b.second;
                return diff;
            })
            parsedFont += 'const kerning_t kernings[] = {\n';
            for (let i = 0; i < fontData.kernings.length; i++) {
                const { first, second, amount} = fontData.kernings[i];
                parsedFont += `\t{${first}, ${second}, ${amount}},\n`;
            }
            parsedFont = parsedFont.slice(0, -2);
            
            parsedFont += '\n};\n\n';
            resolve(parsedFont);
        }
    })
}

function parseImage(file) {
    return new Promise((resolve, reject)=> {
        const image = new Image();
        image.src = URL.createObjectURL(file);
        document.body.appendChild(image);
        const canvas = document.createElement('canvas');
        const ctx = canvas.getContext('2d');
        image.onload = ()=> {
            canvas.width = image.width;
            canvas.height = image.height;
            ctx.drawImage(image, 0, 0, canvas.width, canvas.height);
            const pixelData = ctx.getImageData(0, 0, canvas.width, canvas.height);
            let string = '{';
            for (let i = 0; i < pixelData.data.length; i+=4) {
                string += pixelData.data[i] + ',';
            }
            string = string.slice(0, -1);
            string += '}';
            resolve(string);
        }
    });
}

function loadImages() {
    const promises = [];

    for (let i = 0; i < pages.files.length; i++) {
        promises.push(parseImage(pages.files[i]));
    }

    return new Promise((resolve, reject)=> {
        Promise.all(promises).then((result)=> {
            console.log(result)
            let string = 'const uint8_t image[][] = {';
            for (let i = 0; i < result.length; i++) {
                string += result[i];
            }
            string += '\n};\n';
            resolve(string);
        });
    });
}

convert.addEventListener('click', ()=> {    
    Promise.all([loadImages(), parseFont()]).then((values)=> {
        output.textContent = '';
        // output.textContent += typedefs;
        output.textContent += values[1];
        output.textContent += values[0];
    });
});