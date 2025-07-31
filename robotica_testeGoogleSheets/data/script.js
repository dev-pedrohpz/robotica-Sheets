function enviarComando(cmd) {
    fetch("/comando?acao=" + cmd)
        .then(r => r.text())
        .then(data => document.getElementById("status").innerText = data)
        .catch(() => document.getElementById("status").innerText = "Erro ao enviar comando.");
}

// Atualiza tempo do ciclo automático a cada 2 segundos
setInterval(() => {
    fetch("/tempo")
        .then(r => r.text())
        .then(ms => document.getElementById("tempoCiclo").innerText = ms)
        .catch(() => document.getElementById("tempoCiclo").innerText = "--");
}, 2000);

// Atualiza contador de peças a cada 2 segundos
setInterval(() => {
    fetch("/contador")
        .then(r => r.text())
        .then(valor => document.getElementById("contadorPecas").innerText = valor)
        .catch(() => document.getElementById("contadorPecas").innerText = "--");
}, 2000);

// coleta de dados para a planilha.
// Função para enviar comandos para o ESP32
function enviarComando(cmd) {
    fetch("/comando?acao=" + cmd)
        .then(r => r.text())
        .then(data => document.getElementById("status").innerText = data)
        .catch(() => document.getElementById("status").innerText = "Erro ao enviar comando.");
}

// Array para armazenar os dados coletados
let dadosColetados = [];

// Função que faz fetch no endpoint /dados para pegar dados JSON
function coletarDados() {
    fetch("/dados")
        .then(r => r.json())
        .then(data => {
            document.getElementById("tempoCiclo").innerText = data.tempo;
            document.getElementById("contadorPecas").innerText = data.contador;

            // Armazena localmente (opcional)
            dadosColetados.push({
                timestamp: new Date().toLocaleString(),
                tempo: data.tempo,
                contador: data.contador
            });

            // ENVIA PARA O GOOGLE SHEETS
            fetch("https://script.google.com/macros/s/AKfycbwFoYM3qf92eMw3QkxJKsjzN4hs8Rtm-Gt3VJt521pzgeS0v8lRVFRJxBIE1oV2vr8AkQ/exec", {
                method: "POST",
                body: JSON.stringify({
                    tempo: data.tempo,
                    contador: data.contador
                }),
                headers: {
                    "Content-Type": "application/json"
                }
            });

        })
        .catch(() => {
            document.getElementById("tempoCiclo").innerText = "--";
            document.getElementById("contadorPecas").innerText = "--";
        });
}

// Coleta dados a cada 2 segundos
setInterval(coletarDados, 2000);

// Função para gerar arquivo CSV e disparar download
function baixarCSV() {
    let csv = "Timestamp,Tempo (ms),Contador\n";
    dadosColetados.forEach(dado => {
        csv += `"${dado.timestamp}",${dado.tempo},${dado.contador}\n`;
    });

    const blob = new Blob([csv], { type: "text/csv" });
    const url = URL.createObjectURL(blob);

    const a = document.createElement("a");
    a.href = url;
    a.download = "dados.csv";
    a.click();

    URL.revokeObjectURL(url);
}

// Cria botão “Baixar CSV” ao carregar a página
window.onload = () => {
    const btn = document.createElement("button");
    btn.textContent = "Baixar CSV";
    btn.onclick = baixarCSV;
    document.querySelector(".status-container").appendChild(btn);

    coletarDados(); // coleta inicial imediata
};



// CÓDIGO ATUALIZADO COM MELHORIAS, não deu certo

// function enviarComando(cmd) {
//     fetch("/comando?acao=" + cmd)
//         .then(r => r.text())
//         .then(data => document.getElementById("status").innerText = data)
//         .catch(() => document.getElementById("status").innerText = "Erro ao enviar comando.");
// }

// // Array para armazenar os dados coletados
// let dadosColetados = [];

// // Função que faz fetch no endpoint /dados para pegar dados JSON
// function coletarDados() {
//     fetch("/dados")
//         .then(r => r.json())
//         .then(data => {
//             // Atualiza os valores na interface
//             document.getElementById("tempoCiclo").innerText = data.tempo;
//             document.getElementById("contadorPecas").innerText = data.contador;

//             // Armazena localmente (opcional)
//             dadosColetados.push({
//                 timestamp: new Date().toLocaleString(),
//                 tempo: data.tempo,
//                 contador: data.contador
//             });

//             // ENVIA PARA O GOOGLE SHEETS
//             fetch("https://script.google.com/macros/s/AKfycbwFoYM3qf92eMw3QkxJKsjzN4hs8Rtm-Gt3VJt521pzgeS0v8lRVFRJxBIE1oV2vr8AkQ/exec", {
//                 method: "POST",
//                 body: JSON.stringify({
//                     tempo: data.tempo,
//                     contador: data.contador
//                 }),
//                 headers: {
//                     "Content-Type": "application/json"
//                 }
//             });
//         })
//         .catch(() => {
//             document.getElementById("tempoCiclo").innerText = "--";
//             document.getElementById("contadorPecas").innerText = "--";
//         });
// }

// // Coleta dados a cada 2 segundos
// setInterval(coletarDados, 2000);

// // Função para gerar arquivo CSV e disparar download
// function baixarCSV() {
//     let csv = "Timestamp,Tempo (ms),Contador\n";
//     dadosColetados.forEach(dado => {
//         csv += `"${dado.timestamp}",${dado.tempo},${dado.contador}\n`;
//     });

//     const blob = new Blob([csv], { type: "text/csv" });
//     const url = URL.createObjectURL(blob);

//     const a = document.createElement("a");
//     a.href = url;
//     a.download = "dados.csv";
//     a.click();

//     URL.revokeObjectURL(url);
// }

// // Cria botão “Baixar CSV” ao carregar a página
// window.onload = () => {
//     const btn = document.createElement("button");
//     btn.textContent = "Baixar CSV";
//     btn.onclick = baixarCSV;
//     document.querySelector(".status-container").appendChild(btn);

//     coletarDados(); // coleta inicial imediata
// };