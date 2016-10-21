
// Fireworks effect
// Based on: http://jsfiddle.net/dtrooper/AceJJ/

const MAX_PARTICLES = 10
const SCREEN_WIDTH = window.innerWidth
const SCREEN_HEIGHT = window.innerHeight

function Fireworks() {


    // create canvas
    this.canvas = document.getElementById('fireworks')
    console.log(this.canavas)
    this.context = this.canvas.getContext('2d')
    this.particles = []
    this.rockets = []
    this.MAX_PARTICLES = 400
    this.colorCode = 0
    this.screenWidth = SCREEN_WIDTH

    // Initialize
    //document.body.appendChild(this.canvas);
    this.canvas.width = SCREEN_WIDTH;
    this.canvas.height = SCREEN_HEIGHT;
    //setTimeout(this.start.bind(this), 800);
    this.start()
    setInterval(this.loop.bind(this), 1000 / 50);
}

Fireworks.prototype.start = function() {
    for (var i = 0; i < 5; i++) {
        this.launchFrom(Math.random() * SCREEN_HEIGHT * 2 / 3 + SCREEN_WIDTH / 6);
    }
}

Fireworks.prototype.launchFrom = function(x) {
    if (this.rockets.length < 10) {
        var rocket = new Rocket(x);
        rocket.explosionColor = Math.floor(Math.random() * 360 / 10) * 10;
        rocket.vel.y = Math.random() * -3 - 4;
        rocket.vel.x = Math.random() * 6 - 3;
        rocket.size = 8;
        rocket.shrink = 0.999;
        rocket.gravity = 0.01;
        this.rockets.push(rocket);
    }
}

Fireworks.prototype.loop = function() {
    // update screen size
    //if (SCREEN_WIDTH != window.innerWidth) {
    //    canvas.width = SCREEN_WIDTH = window.innerWidth;
    //}
    //if (SCREEN_HEIGHT != window.innerHeight) {
    //    canvas.height = SCREEN_HEIGHT = window.innerHeight;
   // }

    // clear canvas
    this.context.fillStyle = "rgba(0, 0, 0, 0.05)";
    this.context.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    var existingRockets = [];

    for (var i = 0; i < this.rockets.length; i++) {
        const r = this.rockets[i]
        // update and render
        r.update();
        r.render(this.context);


/* Explosion rules
             - 80% of screen
            - going down
            - close to the mouse
            - 1% chance of random explosion
        */

        if (r.pos.y < SCREEN_HEIGHT / 5 || r.vel.y >= 0 /*|| randomChance*/) {
            r.explode(this.particles);
        } else {
            existingRockets.push(r);
        }

    }
    this.rockets = existingRockets;

    var existingParticles = [];

    for (var i = 0; i < this.particles.length; i++) {
        this.particles[i].update();

        // render and save particles that can be rendered
        if (this.particles[i].exists()) {
            this.particles[i].render(this.context);
            existingParticles.push(this.particles[i]);
        }
    }

    // update array with existing particles - old particles should be garbage collected
    this.particles = existingParticles;

    while (this.particles.length > MAX_PARTICLES) {
        this.particles.shift();
    }
}

function Particle(pos) {
    this.pos = {
        x: pos ? pos.x : 0,
        y: pos ? pos.y : 0
    };
    this.vel = {
        x: 0,
        y: 0
    };
    this.shrink = .97;
    this.size = 2;

    this.resistance = 1;
    this.gravity = 0;

    this.flick = false;

    this.alpha = 1;
    this.fade = 0;
    this.color = 0;
}

Particle.prototype.update = function() {
    // apply resistance
    this.vel.x *= this.resistance;
    this.vel.y *= this.resistance;

    // gravity down
    this.vel.y += this.gravity;

    // update position based on speed
    this.pos.x += this.vel.x;
    this.pos.y += this.vel.y;

    // shrink
    this.size *= this.shrink;

    // fade out
    this.alpha -= this.fade;
};

Particle.prototype.render = function(c) {
    if (!this.exists()) {
        return;
    }

    c.save();

    c.globalCompositeOperation = 'lighter';

    var x = this.pos.x,
        y = this.pos.y,
        r = this.size / 2;

    var gradient = c.createRadialGradient(x, y, 0.1, x, y, r);
    gradient.addColorStop(0.1, "rgba(255,255,255," + this.alpha + ")");
    gradient.addColorStop(0.8, "hsla(" + this.color + ", 100%, 50%, " + this.alpha + ")");
    gradient.addColorStop(1, "hsla(" + this.color + ", 100%, 50%, 0.1)");

    c.fillStyle = gradient;

    c.beginPath();
    c.arc(this.pos.x, this.pos.y, this.flick ? Math.random() * this.size : this.size, 0, Math.PI * 2, true);
    c.closePath();
    c.fill();

    c.restore();
};

Particle.prototype.exists = function() {
    return this.alpha >= 0.1 && this.size >= 1;
};

function Rocket(x) {
    Particle.apply(this, [{
        x: x,
        y: SCREEN_HEIGHT}]);

    this.explosionColor = 0;
}

Rocket.prototype = new Particle();
Rocket.prototype.constructor = Rocket;

Rocket.prototype.explode = function(particles) {
    var count = Math.random() * 10 + 80;

    for (var i = 0; i < count; i++) {
        var particle = new Particle(this.pos);
        var angle = Math.random() * Math.PI * 2;

        // emulate 3D effect by using cosine and put more particles in the middle
        var speed = Math.cos(Math.random() * Math.PI / 2) * 15;

        particle.vel.x = Math.cos(angle) * speed;
        particle.vel.y = Math.sin(angle) * speed;

        particle.size = 10;

        particle.gravity = 0.2;
        particle.resistance = 0.92;
        particle.shrink = Math.random() * 0.05 + 0.93;

        particle.flick = true;
        particle.color = this.explosionColor;

        particles.push(particle);
    }
};

Rocket.prototype.render = function(c) {
    if (!this.exists()) {
        return;
    }

    c.save();

    c.globalCompositeOperation = 'lighter';

    var x = this.pos.x,
        y = this.pos.y,
        r = this.size / 2;

    var gradient = c.createRadialGradient(x, y, 0.1, x, y, r);
    gradient.addColorStop(0.1, "rgba(255, 255, 255 ," + this.alpha + ")");
    gradient.addColorStop(1, "rgba(0, 0, 0, " + this.alpha + ")");

    c.fillStyle = gradient;

    c.beginPath();
    c.arc(this.pos.x, this.pos.y, this.flick ? Math.random() * this.size / 2 + this.size / 2 : this.size, 0, Math.PI * 2, true);
    c.closePath();
    c.fill();

    c.restore();
};


module.exports = Fireworks

