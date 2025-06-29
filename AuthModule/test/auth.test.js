const request = require('supertest');
const { expect } = require('chai');
const jwt = require('jsonwebtoken');
const app = require('../app'); // Adjust the path to your app's entry point

process.env.NODE_ENV = 'test';

describe('POST /login', () => {
  it('should return a JWT for valid credentials', (done) => {
    const userData = {
      userId: 'testuser123',
      roles: ['player', 'tester'],
    };

    request(app)
      .post('/login')
      .send(userData)
      .expect(200)
      .end((err, res) => {
        if (err) return done(err);

        expect(res.body).to.have.property('token');
        const decoded = jwt.decode(res.body.token);
        expect(decoded).to.have.property('userId', userData.userId);
        expect(decoded).to.have.property('roles').deep.equal(userData.roles);
        done();
      });
  });

  it('should return 400 if userId is missing', (done) => {
    const userData = {
      roles: ['player'],
    };

    request(app)
      .post('/login')
      .send(userData)
      .expect(400)
      .end((err, res) => {
        if (err) return done(err);
        expect(res.body).to.have.property('message', 'userId and roles are required');
        done();
      });
  });

    it('should return 400 if roles are missing', (done) => {
    const userData = {
      userId: 'testuser123',
    };

    request(app)
      .post('/login')
      .send(userData)
      .expect(400)
      .end((err, res) => {
        if (err) return done(err);
        expect(res.body).to.have.property('message', 'userId and roles are required');
        done();
      });
  });
}); 