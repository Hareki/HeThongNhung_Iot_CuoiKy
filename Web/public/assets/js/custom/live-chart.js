var gasPPM_60sec = {
    labels: ["60", "59", "58", "57", "56", "55", "54", "53", "52", "51", "50", "49", "48", "47", "46", "45", "44", "43", "42", "41", "40", "39", "38", "37", "36", "35", "34", "33", "32", "31", "30", "29", "28", "27", "26", "25", "24", "23", "22", "21", "20", "19", "18", "17", "16", "15", "14", "13", "12", "11", "10", "9", "8", "7", "6", "5", "4", "3", "2", "1", "0"],
    dataUnit: '',
    lineTension: .3,
    datasets: [{
        label: "Gas PPM last 60 seconds",
        color: "#fff200",
        background: "transparent",
        data: []
    }]
};
//[ 102, 119, 130, 107, 144, 132, 133, 121, 147, 114, 147, 124, 145, 121, 125, 115, 132, 122, 128, 115, 138, 145, 134, 110, 144, 148, 146, 101, 125, 132, 112, 119, 110, 101, 128, 136, 100, 114, 129, 119, 148, 134, 136, 103, 116, 107, 146, 131, 126, 149, 145, 141, 101, 139, 118, 146, 114, 148, 136, 125, 122 ]
var dustDensity_60sec = {
    labels: ["60", "59", "58", "57", "56", "55", "54", "53", "52", "51", "50", "49", "48", "47", "46", "45", "44", "43", "42", "41", "40", "39", "38", "37", "36", "35", "34", "33", "32", "31", "30", "29", "28", "27", "26", "25", "24", "23", "22", "21", "20", "19", "18", "17", "16", "15", "14", "13", "12", "11", "10", "9", "8", "7", "6", "5", "4", "3", "2", "1", "0"],
    dataUnit: '',
    lineTension: .3,
    datasets: [{
        label: "Dust Density last 60 seconds",
        color: "#ff63a5",
        background: "transparent",
        data: []
    }]
};

var temperature_60sec = {
    labels: ["60", "59", "58", "57", "56", "55", "54", "53", "52", "51", "50", "49", "48", "47", "46", "45", "44", "43", "42", "41", "40", "39", "38", "37", "36", "35", "34", "33", "32", "31", "30", "29", "28", "27", "26", "25", "24", "23", "22", "21", "20", "19", "18", "17", "16", "15", "14", "13", "12", "11", "10", "9", "8", "7", "6", "5", "4", "3", "2", "1", "0"],
    dataUnit: '',
    lineTension: .3,
    datasets: [{
        label: "Temperature last 60 seconds",
        color: "#ff63a5",
        background: "transparent",
        data: []
    }]
};

var humidity_60sec = {
    labels: ["60", "59", "58", "57", "56", "55", "54", "53", "52", "51", "50", "49", "48", "47", "46", "45", "44", "43", "42", "41", "40", "39", "38", "37", "36", "35", "34", "33", "32", "31", "30", "29", "28", "27", "26", "25", "24", "23", "22", "21", "20", "19", "18", "17", "16", "15", "14", "13", "12", "11", "10", "9", "8", "7", "6", "5", "4", "3", "2", "1", "0"],
    dataUnit: '',
    lineTension: .3,
    datasets: [{
        label: "Humidity last 60 seconds",
        color: "#33d895",
        background: "transparent",
        data: []
    }]
};
function drawCharts(selector, set_data) {
    var $selector = (selector) ? $(selector) : $('.live-chart');
    $selector.each(function () {
        var $self = $(this), _self_id = $self.attr('id'), _get_data = (typeof set_data === 'undefined') ? eval(_self_id) : set_data;
        var selectCanvas = document.getElementById(_self_id).getContext("2d");
        var chart_data = [];
        for (var i = 0; i < _get_data.datasets.length; i++) {
            chart_data.push({
                label: _get_data.datasets[i].label,
                tension: _get_data.lineTension,
                backgroundColor: _get_data.datasets[i].background,
                borderWidth: 2,
                borderColor: _get_data.datasets[i].color,
                pointBorderColor: 'transparent',
                pointBackgroundColor: 'transparent',
                pointHoverBackgroundColor: "#fff",
                pointHoverBorderColor: _get_data.datasets[i].color,
                pointBorderWidth: 2,
                pointHoverRadius: 4,
                pointHoverBorderWidth: 2,
                pointRadius: 4,
                pointHitRadius: 4,
                data: _get_data.datasets[i].data,
            });
        }
        var chart = new Chart(selectCanvas, {
            type: 'line',
            data: {
                labels: _get_data.labels,
                datasets: chart_data,
            },
            options: {
                legend: {
                    display: (_get_data.legend) ? _get_data.legend : false,
                    rtl: NioApp.State.isRTL,
                    labels: {
                        boxWidth: 12,
                        padding: 20,
                        fontColor: '#6783b8',
                    }
                },
                maintainAspectRatio: false,
                tooltips: {
                    enabled: true,
                    rtl: NioApp.State.isRTL,
                    callbacks: {
                        title: function (tooltipItem, data) {
                            return false;
                        },
                        label: function (tooltipItem, data) {
                            return data.datasets[tooltipItem.datasetIndex]['data'][tooltipItem['index']] + ' ' + _get_data.dataUnit;
                        }
                    },
                    backgroundColor: '#1c2b46',
                    titleFontSize: 8,
                    titleFontColor: '#fff',
                    titleMarginBottom: 4,
                    bodyFontColor: '#fff',
                    bodyFontSize: 8,
                    bodySpacing: 4,
                    yPadding: 6,
                    xPadding: 6,
                    footerMarginTop: 0,
                    displayColors: false
                },
                scales: {
                    yAxes: [{
                        display: false,
                        ticks: {
                            beginAtZero: false,
                            fontSize: 12,
                            fontColor: '#9eaecf',
                            padding: 5
                        },
                        gridLines: {
                            color: NioApp.hexRGB("#526484", .2),
                            tickMarkLength: 0,
                            zeroLineColor: NioApp.hexRGB("#526484", .2)
                        },
                    }],
                    xAxes: [{
                        display: false,
                        ticks: {
                            fontSize: 12,
                            fontColor: '#9eaecf',
                            source: 'auto',
                            padding: 5,
                            reverse: true
                        },
                        gridLines: {
                            color: "transparent",
                            tickMarkLength: 0,
                            zeroLineColor: NioApp.hexRGB("#526484", .2),
                            offsetGridLines: true,
                        }
                    }]
                },
                animation: {
                    duration: 0
                }

            }
        });
    })
}

function getRndInteger(min, max) {
    return Math.floor(Math.random() * (max - min)) + min;
}

function updateChart(chart) {
    var array, length, value;
    switch (chart) {
        case 'gasPPM':
            array = gasPPM_60sec.datasets[0].data;
            value = vGasPPM;
            break;
        case 'humidity':
            array = humidity_60sec.datasets[0].data;
            value = vHumidity;
            break;
        case 'temperature':
            array = temperature_60sec.datasets[0].data;
            value = vTemperature
            break;
        case 'dustDensity':
            array = dustDensity_60sec.datasets[0].data;
            value = vDustDensity;
            break;
    }
    length = array.length;
    array.unshift(value);
    console.log(chart + ': ' + value);
    if (length > 61) {
        array.pop();
    }
}
setInterval(function () {
    updateChart('gasPPM');
    updateChart('humidity');
    updateChart('temperature');
    updateChart('dustDensity');
    drawCharts();
}, 1000);



